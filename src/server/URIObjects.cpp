#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <experimental/filesystem>
#include <chrono>
#include <folly/io/async/EventBaseManager.h>
#include <folly/FileUtil.h>
#include <folly/executors/GlobalExecutor.h>
#include "src/server/URIObjects.hpp"
#include "src/zotero/zotero.hpp"

using namespace proxygen;
namespace fs = std::experimental::filesystem;

void UserSystemHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	//Only if the user is logged in send him the information about his profile
	if(!_user)
		return SendErrorNotFound(downstream_);

	//Check if the user wants to get his profile information or if he wants to get information about all users
	if(headers->getPath() == "/getProfileInfo")
	{
		//Send the information about the user downstream to the client
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(_user->toJSON()))
			.sendWithEOM();
		return;
	}
	else if(headers->getPath() == "/userList")
	{
		//Check if the user has the necessary access rights to get the list of all users
		if(!User::AccessCheck(_user,AccessRights::USR_ADMIN))
			return SendAccessDenied(downstream_);

		//Ok he got the necessary access rights so send the list of all users back to the user in the json format
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(UserHandler::GetUserTable().toJSON()))
			.sendWithEOM();
		return;
	}
	else return SendErrorNotFound(downstream_); //Something entirly wrong happened here
}




void UpdateUserSystemHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept
{
	//Check if the user has admin access
	if(!User::AccessCheck(_user,AccessRights::USR_ADMIN))	
		return SendAccessDenied(downstream_);

	try
	{
		//Try to parse the command list from the body received by zotero
		nlohmann::json js = nlohmann::json::parse(std::string(reinterpret_cast<const char*>(body->data()),body->length()));

		//Do the updates accordingly
		for(auto &it : js)
		{
			if(it["action"]=="DELETE")
			{
				//Remove the user if the action is delete if one of the necessary variables does not exist then throw an error and return an error not found
				UserHandler::GetUserTable().RemoveUser(it["email"]);
			}
			else if(it["action"]=="CREATE")
			{
				//Create the user with the specified email password and access
				UserHandler::GetUserTable().AddUser(it["email"],it["password"],it["access"]);
			}
			else if(it["action"]=="CHANGEACCESS")
			{
				//Create the user with the given access rights
				UserHandler::GetUserTable().SetAccessRights(it["email"],it["access"]);
			}
		}
		//Tell the client about the successful completed operations
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.sendWithEOM();
	}
	catch(...)
	{
		//Ok something went wrong here just send back an error message to the client
		SendErrorNotFound(downstream_,"<h1>Received corrupted json!</h1>");
	}
}

/**
 * @brief Creates a string which contains all file names in a big json file
 *
 * @param path the path to the directory to create a json from
 *
 * @return The json of all file names packed in one json
 */
std::string DirectoryFilesToJSON(const std::string &path)
{
	//Check if the directory exists if not return an empty json
	if (fs::exists(path) && fs::is_directory(path))
	{
		//Start a list of jsons
		std::string js = "[";
		for(const auto &dirEntry : fs::directory_iterator(path))
		{
			//Create for every entry in the directory an entry in the json
			js+="{\"name\":\"";
			js+=dirEntry.path().filename();
			//The json contains at the moment only the filename
			js+="\"},";
		}
		//Replace the last , by and ] to close the enumeration of files
		js[js.length()-1]=']';
		//Return the json we just created
		return std::move(js);
	}
	return "";
}


void GetBookRessource::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	try
	{
		//Get the book part of the request as well as the ressource part of the request
		std::string book = headers->getDecodedQueryParam("book");
		std::string res = headers->getDecodedQueryParam("ressource");

		if(book.find("..")!=std::string::npos||book.find("~")!=std::string::npos
				||res.find("..")!=std::string::npos||res.find("~")!=std::string::npos)
			return SendAccessDenied(downstream_);

		//If book and res is zero the client wants to get a list of all books on the server
		if(book==""&&res=="")
		{
			auto &mapBooks = GetSearchHandler::GetBookManager().getMapOfBooks();
			nlohmann::json ret;
			for(auto &it : mapBooks)
			{
				nlohmann::json x;
				x["bib"] = it.second.getMetadata().getMetadata()["bib"];
				x["key"] = it.second.getKey();
				ret.push_back(std::move(x));
			}
			//Return a list of all books with bibliography and ocr yes false
			return ResponseBuilder(downstream_)
				.status(200,"Ok")
				.header("Content-Type","application/json")
				.body(ret.dump())
				.sendWithEOM();
			return;
		}
		else if(book!=""&&res=="") //Returns a list of all files in a specific book
		{
			//Construct the path to the book we want to have informations about
			std::string path = "web/books/";
			path+=book;
			//Get a json with all files in the directory
			std::string js = std::move(DirectoryFilesToJSON(path));
			//if it is zero the directory does not exist so there are no files on the server which are connected to the specified book
			if(js=="")
				throw 0;

			//Ok we got the list with files so build the fitting response and send it back
			ResponseBuilder(downstream_)
				.status(200,"Ok")
				.header("Content-Type","application/json")
				.body(std::move(js))
				.sendWithEOM();
		}
		else if(book!=""&&res!="") //Reads the specific ressource and sends the file with a specific mime type to the client
		{
			
			//Create the path to the book
			std::string path = "web/books/";
			path+=book;
			path+="/";
			path+=res;

			if(headers->getDecodedQueryParam("exist_check")!="")
			{
				if(fs::exists(path))
				{
					return ResponseBuilder(downstream_)
						.status(200,"Ok")
						.body("file exists")
						.sendWithEOM();
				}
				else
				{
					return SendErrorNotFound(downstream_);
				}
			}
			std::thread([mypath=std::move(path),this,evb=folly::EventBaseManager::get()->getEventBase()]{

			bool onerror = false;
			URIFile *fl;
			try
			{
				//Load the ressource from the constructed path if the ressource does not exist the constructor of URIFile will throw an expection 
				fl = new URIFile(std::move(mypath));
			}
			catch(...)
			{
				onerror = true;
			}

			evb->runInEventBaseThread([fl,onerror,this]{
			if(onerror)
				return SendErrorNotFound(downstream_);
			//So we know the file is good now send back the file with the detected mime type
			ResponseBuilder(downstream_)
				.status(200,"Ok")
				.header("Content-Type",fl->getMimeType())
				.body(std::move(fl->getBufferReference()))
				.sendWithEOM();
				delete fl;
				});

				}).detach();
		}
	}
	catch(...)
	{
		//Something did not work send back an error not found!
		return SendErrorNotFound(downstream_);
	}
}



void GetSearchHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	try
	{
		alx::cout<< headers->getURL() <<alx::endl;
		alx::cout<< headers->getDecodedQueryParam("pillars") <<alx::endl;
		std::string word = headers->getDecodedQueryParam("search");
		std::string author = headers->getDecodedQueryParam("author");
		int Fuzzyness = headers->getIntQueryParam("fuzzyness",0);
		int from = headers->getIntQueryParam("publicatedafter",0);
		int to = headers->getIntQueryParam("publicatedbefore",0);
		bool onlyTitle = false;
		bool ocrOnly = false;
		std::vector<std::string> pillars;
		std::stringstream ss(headers->getDecodedQueryParam("pillars"));
		std::string fill;
		while(getline(ss,fill,','))
			if(fill!="")
				pillars.push_back(std::move(fill));

		if(headers->getDecodedQueryParam("title_only")=="true")
			onlyTitle = true;
		if(headers->getDecodedQueryParam("ocr_only")=="true")
			ocrOnly = true;
		alx::cout<<"Search for: "<<word<<" author: "<<author<<" Fuzzyness: "<<Fuzzyness<<" released before: "<<from<<" and after: "<<to<<"\nSearch only title "<<onlyTitle<<alx::endl<<"Search only ocr: "<<ocrOnly<<alx::endl;
		alx::cout<<"Pillars: ";
		for(auto &it : pillars)
		{
			alx::cout<<it<<",";
		}
		alx::cout<<alx::endl;

		CSearchOptions *nso = new CSearchOptions(word,Fuzzyness,std::move(pillars),onlyTitle,ocrOnly,std::move(author),from,to,true);
		
		static std::atomic<unsigned long long> unique_sid = 0;
		long long searchid = unique_sid.fetch_add(1);
		CSearch * csearch = new CSearch(nso,searchid);
		GetSearchHandler::GetBookManager().addSearch(csearch);
		nlohmann::json json;
		json["searchid"] = std::to_string(searchid);
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(json.dump()))
			.sendWithEOM();
		return;
	}
	catch(...)
	{
		return SendAccessDenied(downstream_);
	}
}

CBookManager &GetSearchHandler::GetBookManager()
{
	static CBookManager gManager;
	return gManager;
}


void GetSearchInBookHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	try
	{
		std::string searchFor = headers->getDecodedQueryParam("query");
		std::string inBook = headers->getDecodedQueryParam("scanId");
		int Fuzzyness = headers->getIntQueryParam("fuzzyness",0);
		if(searchFor==""||inBook=="")
			throw 0;
		auto &mapofbooks = GetSearchHandler::GetBookManager().getMapOfBooks();
		auto &book = mapofbooks.at(inBook);
		alx::cout.write(alx::console::green_black,"New search request! Searching for: ",searchFor,"\n");


		nlohmann::json js;

		auto start = std::chrono::system_clock::now();
		if(Fuzzyness==0)
		{
			alx::cout.write(alx::console::yellow_black,"Using normal search!\n");
			std::unique_ptr<std::list<size_t>> listPtr(book.getPagesFull(std::move(searchFor)));

			js["is_fuzzy"] = false;
			for(auto it : *listPtr)
			{
				int k = it;
				js["books"].push_back(k);
			}
		}
		else
		{
			std::unique_ptr<std::map<int,std::vector<std::string>>> mapPtr;

			if(Fuzzyness==1)
			{

				alx::cout.write(alx::console::yellow_black,"Using contains search!\n");
				mapPtr = std::unique_ptr<std::map<int,std::vector<std::string>>>(book.getPagesContains(std::move(searchFor)));
			}
			else
			{

				alx::cout.write(alx::console::yellow_black,"Using fuzzy search!\n");
				mapPtr = std::unique_ptr<std::map<int,std::vector<std::string>>>(book.getPagesFuzzy(std::move(searchFor)));
			}

			js["is_fuzzy"] = true;
			auto glambda = [](std::string const &str, std::string const &from,std::string const &to) -> std::string {return std::regex_replace(str,std::regex(from),to);};
			for(auto const &it : *mapPtr)
			{
				for(auto const &inner : it.second)
				{
					nlohmann::json entry;

					entry["page"] = it.first;
					entry["word"] = glambda(inner,"\"","\\\"");
					js["books"].push_back(std::move(entry));
				}
			}
		}
		auto end=std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		alx::cout.write(alx::console::yellow_black,"Search in book time: ",elapsed_seconds.count(),"s \n");
		return ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(js.dump()))
			.sendWithEOM();
	}
	catch(...)
	{
		return SendErrorNotFound(downstream_);
	}
}

void RedirectToHTTPSHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> /*headers*/) noexcept
{
	ResponseBuilder(downstream_)
		.status(301,"Moved Permanently")
		.header("Location","https://www.clas-digital.uni-frankfurt.de")
		.sendWithEOM();
}

void GetBookMetadata::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	try
	{
		auto &mapofbooks = GetSearchHandler::GetBookManager().getMapOfBooks();
		std::string inBook = headers->getDecodedQueryParam("scanId");
		
		if(inBook=="") throw 0;

		std::string json;
		try
		{
			json = std::move(mapofbooks.at(inBook).getMetadata().getMetadata().dump());
		}
		catch(...)
		{
			json = Zotero::SendRequest(Zotero::Request::GetSpecificItem(inBook));
			
			if(json=="") throw 0;

			nlohmann::json js;
			js.push_back(std::move(nlohmann::json::parse(json)));

			alx::cout.writeln("Adding a new json to the map of all books with key: ",alx::console::yellow_black,js[0]["data"]["key"].get<std::string>());
			GetSearchHandler::GetBookManager().updateZotero(std::move(js));
		}
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(json))
			.sendWithEOM();
	}
	catch(...)
	{
		return SendErrorNotFound(downstream_);
	}
}


void UploadBookHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	try
	{
		if(!User::AccessCheck(_user,AccessRights::USR_WRITE))
			throw 0;

		std::string whichBook = headers->getDecodedQueryParam("itemKey");
		std::string filename = headers->getDecodedQueryParam("filename");

		if(filename.find("..")!=std::string::npos||filename.find("~")!=std::string::npos)
			throw "Illegal filename provided!";

		auto &mapofbooks = GetSearchHandler::GetBookManager().getMapOfBooks();
		mapofbooks.at(whichBook); //Check if book exists
		_finalPath = "web/books/";
		_finalPath+=whichBook;
		if(!fs::exists(_finalPath))
		{
			fs::create_directory(_finalPath);
		}

		_finalPath+="/";
		_finalPath+=filename;

		if(fs::exists(_finalPath))
			throw "File or folder already exists!";

		ofs.open(_finalPath.c_str(),std::ios::out);
		
		if(!ofs.is_open())
			throw "Cannot open path to write the file!";
	}
	catch(const char *msg)
	{
		return SendAccessDenied(downstream_,msg);
	}
	catch(...)
	{
		return SendAccessDenied(downstream_);
	}
}

void UploadBookHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept
{
	if(ofs.is_open())
		ofs.write(reinterpret_cast<const char*>(body->data()),body->length());
}


void UploadBookHandler::onEOM() noexcept
{
	if(ofs.is_open())
	{
		ofs.close();
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.body("Successfully written file!")
			.sendWithEOM();
	}
}

void StartSearch::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	std::string inBook = headers->getDecodedQueryParam("searchID");
	if(inBook=="")
		return SendErrorNotFound(downstream_);

	long long id = 0;
	try
	{
		id = std::stoll(inBook);
	}
	catch(...)
	{
		return SendErrorNotFound(downstream_);
	}

	std::thread(std::bind(&StartSearch::start,this,id,folly::EventBaseManager::get()->getEventBase())).detach();
}

void StartSearch::start(long long id, folly::EventBase *evb)
{
	alx::cout.write(alx::console::green_black,"Started additional thread for searching!\n");
	auto start = std::chrono::system_clock::now();
	auto results = GetSearchHandler::GetBookManager().search(id);
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	alx::cout.write(alx::console::yellow_black,"Search all books time: ",elapsed_seconds.count(),"s \n");
		nlohmann::json js;

		for(auto &it : *results)
		{
			nlohmann::json entry;
			entry["scanId"] = it.first;
			entry["hasocr"] = it.second->getOcr();
			entry["description"] = it.second->getMetadata().getShow();
			js["books"].push_back(std::move(entry));
		}

		alx::cout.writeln(alx::console::green_black,"Searching done! Number of results: ",results->size());
		delete results;

		evb->runInEventBaseThread([this,json = std::move(js)]{
		alx::cout.write(alx::console::green_black,"Sending search response...\n");
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(json.dump()))
			.sendWithEOM();});
}

void RequestSearchProgress::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	std::string inBook = headers->getDecodedQueryParam("searchID");
	long long id;
	float prog = 1.0;
	try
	{
		id = std::stoll(inBook);
		prog = GetSearchHandler::GetBookManager().getProgress(id);
	}
	catch(...)
	{}

	nlohmann::json j;
	j["progress"] = prog;
	ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(j.dump())
			.sendWithEOM();
}
