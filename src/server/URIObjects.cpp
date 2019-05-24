#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <experimental/filesystem>
#include "src/server/URIObjects.hpp"
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
	//Check if the user has the enough rights to perform the request
	if(!User::AccessCheck(_user,AccessRights::USR_READ))
		return SendAccessDenied(downstream_);

	try
	{
		//Get the book part of the request as well as the ressource part of the request
		std::string book = headers->getDecodedQueryParam("book");
		std::string res = headers->getDecodedQueryParam("ressource");
		//If book and res is zero the client wants to get a list of all books on the server
		if(book==""&&res=="")
		{
			//Return a list of all books with bibliography and ocr yes false
			return SendErrorNotFound(downstream_);
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

			//Load the ressource from the constructed path if the ressource does not exist the constructor of URIFile will throw an expection 
			URIFile fl(std::move(path));

			//So we know the file is good now send back the file with the detected mime type
			ResponseBuilder(downstream_)
				.status(200,"Ok")
				.header("Content-Type",fl.getMimeType())
				.body(std::move(fl.getBufferReference()))
				.sendWithEOM();
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
		if(!User::AccessCheck(_user,AccessRights::USR_READ))
			throw 0;

		std::cout<< headers->getURL() <<std::endl;
		std::cout<< headers->getDecodedQueryParam("pillars") <<std::endl;
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
		std::cout<<"Search for: "<<word<<" author: "<<author<<" Fuzzyness: "<<Fuzzyness<<" released before: "<<from<<" and after: "<<to<<"\nSearch only title "<<onlyTitle<<std::endl<<"Search only ocr: "<<ocrOnly<<std::endl;
		std::cout<<"Pillars: ";
		for(auto &it : pillars)
		{
			std::cout<<it<<",";
		}
		std::cout<<std::endl;
		return SendErrorNotFound(downstream_);
	}
	catch(...)
	{
		return SendAccessDenied(downstream_);
	}
}
