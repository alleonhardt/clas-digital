#include <iostream>
#include <streambuf>
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include "httplib.h"
#include "login/user_system.hpp"
#include "util/URLParser.hpp"
#include "zotero/zotero.hpp"
#include "books/CBookManager.hpp"
#include <signal.h>
#include <chrono>
#include <exception>
#include <string_view>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //Neeeded for image dimensions
#include <sstream>
#include "ocr/tesseract.hpp"
#include "util/StaticWebpageCreator.hpp"


using namespace httplib;

//Defining the srv global to stop the execution once the SIGTERM signal is caught.
Server srv;
OcrCreator ocr_reader;

std::mutex gGlobalUpdateOperationLock;
enum class GlobalUpdateOperations { none, update_zotero, restart_server };
GlobalUpdateOperations gGlobalUpdateOperation = GlobalUpdateOperations::none;
bool gGlobalSigShutdown = false;


void sig_handler(int)
{
    //Just stop the server when systemd wants to exit us.
    srv.stop();
    gGlobalSigShutdown = true;
}

void update_all_files(CBookManager *m, nlohmann::json *zot)
{

    auto &manager = *m;
    auto &zoteroPillars = *zot;

    int managedBooks = 0;

    
    if(!std::filesystem::is_regular_file("updated_static_books.txt"))
    {
#define WRITE_FORBIDDEN_FILES 0
#ifdef WRITE_FORBIDDEN_FILES
	std::string file_content = "";
	std::cout<<"Processed: "<<managedBooks<<"/"<<manager.getMapOfBooks().size()<<std::endl;
#endif
	for(auto it : manager.getMapOfBooks())
	{
	    ++managedBooks;
	    if(managedBooks%200 == 0)
		std::cout<<"Processed: "<<managedBooks<<"/"<<manager.getMapOfBooks().size()<<std::endl;
	    for(auto &it2 : zoteroPillars)
	    {
		if(func::in(it2["key"],it.second->getMetadata().getCollections()))
		{
		    if(it2.count("books_managed")>0)
			it2["books_managed"] = it2["books_managed"].get<int>()+1;
		    else
			it2["books_managed"]=1;
		}
	    }

	    if(gGlobalSigShutdown)
		return;

	    if(it.second->hasContent() && (it.second->getPublic()))
	    {
		std::string loc = "location ~ /books/";
		loc+=it.first;
		loc+="/(?!backups).* {\n\ttry_files $uri $uri/ =404;\n}\n";
#ifdef WRITE_FORBIDDEN_FILES
		// single write to reduce likelyhood of breaking webserver
		// https://github.com/ShadowItaly/clas-digital/issues/175
		file_content+=loc;
#endif
	    }
	    std::error_code ec;
	    std::string dir = "web/books/";
	    dir+=it.first;


	    std::string command = "mkdir -p ";
	    command+=dir;
	    int x = system(command.c_str());
	    x+=1;
	    dir+="/info.json";
	    std::ofstream json_write(dir.c_str(),std::ios::out);
	    if(json_write.is_open())
		json_write<<it.second->getMetadata().getMetadata();
	    json_write.close();

	    StaticWebpageCreator webpage(it.second);
	    webpage.createWebpage(zoteroPillars);
	}
#ifdef WRITE_FORBIDDEN_FILES
	std::ofstream of("bin/tmpforbiddenfiles",std::ios::out);
	of<<file_content;
	of.close();
	std::error_code ec;
	std::filesystem::remove("bin/forbiddenfiles",ec);
	std::filesystem::create_symlink("tmpforbiddenfiles","bin/forbiddenfiles",ec);
	if(ec)
	{
	    std::cout<<"Could not create symlink from the tmpfile to the forbiddenfiles nginx file"<<std::endl;
	}
	else
	    std::cout<<"Update finished all pages are updated now!"<<std::endl;
	int y =system("systemctl restart nginx");
	y++;
#endif

	std::ofstream is_updated("updated_static_books.txt");
	is_updated<<"0";
	is_updated.close();
    }
    else
    {
	std::cout<<"Using cached static files, to rebuild all files use \"make reset_static\""<<std::endl;
    }
    if(!std::filesystem::is_regular_file("updated_catalogue.txt"))
    {
	StaticCatalogueCreator ct;
	ct.CreateCatalogue(); 
	ct.CreateCatalogueAuthors(manager);
	ct.CreateCatalogueBooks(manager);
	ct.CreateCatalogueAuthor(manager);
	ct.CreateCatalogueCollections(zoteroPillars);
	ct.CreateCatalogueCollection(manager, zoteroPillars);
	ct.CreateCatlogueYears(manager);
	std::ofstream ofs("updated_catalogue.txt");
	ofs<<"0";
	ofs.close();
    }

}

void update_and_restart()
{
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t (system_clock::now());

    struct std::tm * ptm = std::localtime(&tt);
    ptm->tm_min = 59;
    ptm->tm_sec = 30;
    ptm->tm_hour = 23;
    std::this_thread::sleep_until (system_clock::from_time_t (mktime(ptm)));

    std::cout << "Current time: " << std::put_time(ptm,"%X") << ".\nUpdating zotero now."<<std::endl;;
    std::cout << "Scheduled update is going to be performed." << std::endl;


    std::unique_lock<std::mutex> lock(gGlobalUpdateOperationLock, std::try_to_lock);
    if(!lock.owns_lock()){
	std::cout<<"Did not update zotero because the server either restarts at the moment, or zotero is being updated!"<<std::endl;
	return;
    }

    gGlobalUpdateOperation = GlobalUpdateOperations::update_zotero;

    try
    {
	//Update the pillars first
	nlohmann::json zot = Zotero::GetPillars();
	std::ofstream of("web/pillars.json",std::ios::out);
	of<<zot;
	of.close();

	nlohmann::json metaData;
	for(auto &it : zot)
	{
	    auto entryjs = nlohmann::json::parse(Zotero::SendRequest(Zotero::Request::GetAllItemsFromCollection(it["key"])));
	    for(auto &it : entryjs)
		metaData.push_back(it);
	}
	//Save the collected data as it is the newest data available
	std::ofstream o("bin/zotero.json",std::ios::out);
	o<<metaData;
	o.close();
	std::cout << "Updated zotero successfully." << std::endl;
    }
    catch(...)
    {
	std::cerr<<"[Server error] Could update zotero, maybe no internet connection?"<<std::endl;
    }

    std::cout<<"Restarting server now!"<<std::endl;
    gGlobalUpdateOperation = GlobalUpdateOperations::restart_server;
    srv.stop();
    std::error_code ec;
    std::filesystem::remove("updated_static_books.txt",ec);
    std::filesystem::remove("updated_catalogue.txt",ec);
}



void do_createbiblio(const Request &req,Response &resp,CBookManager &manager)
{
    try
    {
	std::string inBook = req.get_param_value("books");
	std::string_view inBookView(inBook);
	if(inBook=="")
	{resp.set_content("{}","application/json");return;}

	std::string retval="<html><head><meta charset=\"utf-8\"></head><body>";
	auto &mapBooks = manager.getMapOfBooks();

	size_t x_new = 0;
	size_t x_last = 0;
	for(;;)
	{
	    x_new = inBookView.find(",",x_last);
	    std::string key;
	    if(x_new == std::string::npos)
		key = inBookView.substr(x_last);
	    else
		key = inBookView.substr(x_last,x_new-x_last);

	    try
	    {
		auto &book = mapBooks[key];
		retval+= "<p>";
		retval+= book->getMetadata().getMetadata()["citation"];
		retval+="</p>";
	    }
	    catch(...) {}

	    if(x_new==std::string::npos) break;
	    x_last = x_new+1;
	}
	retval+="</body></html>";
	resp.set_content(retval.c_str(),"text/html");
    }
    catch(...)
    {
	resp.set_content("{}","application/json");return;
    }
}

void get_sugg(const Request &req, Response &resp, CBookManager &manager, std::string sType)
{
    try
    {
	std::string sInput = req.get_param_value("q");
	std::list<std::string>* listSugg = manager.getSuggestions(sInput, sType);
	nlohmann::json responseJson;
	for(auto it=listSugg->begin(); it!=listSugg->end();it++)
	    responseJson.push_back((*it)); 
	delete listSugg;
	resp.set_content(responseJson.dump(),"application/json");
    }

    catch(...) {
	resp.set_content("{}","application/json");
    }
}
void get_metadata(const Request &req, Response &resp, CBookManager &manager)
{
    try
    {
	std::string scanId = req.get_param_value("scanId");
	CBook *b = manager.getMapOfBooks().at(scanId);
	nlohmann::json js = b->getMetadata().getMetadata();
	js["has_ocr"] = b->hasOcr();
	resp.set_content(std::move(js.dump()),"application/json");
    }
    catch(...)
    {
	resp.set_content("{}","application/json");
    }
}

void do_login(const Request& req, Response &resp)
{
    URLParser parser(req.body);
    auto &usrtable = UserHandler::GetUserTable();
    auto ret = usrtable.DoLogin(parser["email"],parser["password"]); //Try to login the user with the provided credentials
    if(ret!="")
    {
	//Tell the user to reload the page
	resp.set_content("<html><head><script>window.location='/';</script></head><body></body></html>","text/html");
	std::string cookie = "SESSID="+ret;
	cookie+="; SECURE";
	resp.set_header("Set-Cookie",cookie.c_str());
    }
    else
    {
	//Show the user the access denied formula
	resp.set_content("<html><head></head><body><h1>Access Denied 403</body></html>","text/html");
	resp.status = 403;
    }
}

void own_split(const std::string &pill, char c, std::vector<std::string> &vec)
{	    auto start = 0;
    std::string tmp;
    while(true)
    {
	auto pos = pill.find(c,start);
	if(pos==std::string::npos)
	{
	    tmp = pill.substr(start);
	}
	else
	{
	    tmp = pill.substr(start,pos);
	}
	if(tmp!="")
	    vec.push_back(std::move(tmp));
	if(pos==std::string::npos)
	    break;
	start = pos+1;
    }
}

void do_searchinbook(const Request& req, Response &resp, CBookManager &manager)
{
    try
    {
	std::string scanId = req.get_param_value("scanId");
	std::string query = req.get_param_value("query");
	int Fuzzyness = std::stoi(req.get_param_value("fuzzyness"));

	std::unique_ptr<std::map<int,std::vector<std::string>>> mapPtr;

	auto book = manager.getMapOfBooks().at(scanId);
	nlohmann::json js;
	js["is_fuzzy"] = true;

	if(Fuzzyness==0) {js["is_fuzzy"] = false;}



	func::convertToLower(query);
	query = func::convertStr(query);
	std::cout<<"Search in book with query: "<<query<<std::endl;
	std::cout<<"And fuzzyness: "<<Fuzzyness<<std::endl;
	mapPtr = std::unique_ptr<std::map<int,std::vector<std::string>>>(book->getPages(std::move(query),Fuzzyness!=0));
	std::cout<<"Got "<<mapPtr->size()<<" search results found!"<<std::endl;

	auto glambda = [](std::string const &str, std::string const &from,std::string const &to) -> std::string {return std::regex_replace(str,std::regex(from),to);};
	for(auto const &it : *mapPtr)
	{
	    nlohmann::json entry;
	    entry["page"] = it.first;
	    std::string val = "";

	    for(auto const &inner : it.second)
	    {
		if(val!="")
		    val+=",";
		val += glambda(inner,"\"","\\\"");
	    }

	    entry["words"] = val;
	    js["books"].push_back(std::move(entry));	
	}
	resp.set_content(js.dump(),"application/json");
    }
    catch(std::exception &e)
    {
	std::cout<<"Caught exception in search in book: "<<e.what()<<std::endl;
	resp.set_content("{}","application/json");
    }
}

void do_search(const Request& req, Response &resp, const std::string &fileSearchHtml, const nlohmann::json &zoteroPillars, CBookManager &manager)
{
    //Is this a search query or just a request for the search main page?
    if(req.has_param("q"))
    {
	try
	{
	    std::string query;
	    bool fuzz = false;
	    std::string scope ="all";
	    std::string author = "";
	    int pubafter = 1700;
	    int pubbef = 2049;
	    std::vector<std::string> pillars;
	    std::string debugpill;
	    int resultsperpage = 10;

	    for(auto &it : zoteroPillars)
	    {
		debugpill+=it["key"];
		debugpill+=";";
		pillars.push_back(it["key"]);
	    }
	    int list_start = 0;
	    std::string sort = "relevance";


	    //This is the only required parameter
	    query = req.get_param_value("q",0);
	    if(req.has_param("fuzzyness")) fuzz = std::stoi(req.get_param_value("fuzzyness",0))!=0;
	    if(req.has_param("scope")) scope = req.get_param_value("scope",0);
	    if(req.has_param("author")) author = req.get_param_value("author",0);
	    if(req.has_param("pillars"))
	    {
		pillars.clear();
		debugpill = req.get_param_value("pillars",0);
		own_split(debugpill,',',pillars);
	    }

	    try{pubafter = std::stoi(req.get_param_value("publicatedafter"));}catch(...){};
	    try{pubbef = std::stoi(req.get_param_value("publicatedbefore"));}catch(...){};
	    try{list_start= std::stoi(req.get_param_value("start"));}catch(...){};
	    try{sort = req.get_param_value("sorting",0);}catch(...){};
	    try{resultsperpage = std::stoi(req.get_param_value("limit",0));}catch(...){};


	    std::cout<<"Receveived search request!"<<std::endl;
	    std::cout<<"Query: "<<query<<"; fuzz: "<<fuzz<<"; scope: "<<scope<<"author: "<<author<<"; publicated after: "<<pubafter<<"; publicated before: "<<pubbef<<"; searched pillars: "<<debugpill<<"; vector pillar size: "<<pillars.size()<<"; sorting with value: "<<sort<<" and max results per page: "<<resultsperpage<<std::endl;


	    CSearchOptions options(query,fuzz,pillars,scope,author,pubafter,pubbef,true,sort);
	    std::cout<<"Starting search!"<<std::endl;

	    auto start = std::chrono::system_clock::now();
	    auto bklst = manager.search(&options);
	    std::cout<<"Finished searching constructing response json"<<std::endl;
	    nlohmann::json js;

	    auto &map = manager.getMapOfBooks();
	    auto iter = bklst->begin();
	    if(bklst->size()<=(static_cast<unsigned int>(list_start)))
	    {
		js["max_results"] = 0;
		js["time"] = 0;
	    }
	    else
	    {

		std::advance(iter,list_start);
		int counter = 0;
		for(;iter!=bklst->end();++iter)
		{
		    auto &it = map.at(*iter);
		    nlohmann::json entry;
		    entry["scanId"] = it->getKey();
		    entry["copyright"] = !it->getPublic();
		    entry["hasocr"] = it->hasContent();
		    entry["description"] = it->getAuthorDateScanned();
		    entry["bibliography"] = it->getMetadata().getMetadata("bib");
		    entry["preview"] = it->getPreview(options.getSearchedWord());
		    js["books"].push_back(std::move(entry));
		    if(++counter==resultsperpage)
			break;
		} 

		js["max_results"] = bklst->size();
		std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
		js["time"] = elapsed_seconds.count();
	    }
	    std::cout<<"Finished constructing json response. Constructing html response!"<<std::endl;
	    std::string app = fileSearchHtml;
	    app+="<script>let ServerDataObj = {pillars:";
	    app+=zoteroPillars.dump();
	    app+=", search:";
	    app+=js.dump();
	    app+="};</script>";
	    resp.set_content(app.c_str(),"text/html");
	}
	catch(std::exception &e)
	{
	    std::cout<<"Caught exception in search all books: "<<e.what()<<std::endl;
	    resp.status = 400;
	    // Custom message might not be needed if "400 Bad Request" can be returned and front-end webserver can do the rest?
	    resp.set_content("<html><head></head><body><h1>Corrupted search request!</h1></body></html>","text/html");
	    return;
	}
    }
    else
    {

	std::string app = fileSearchHtml;
	app+="<script>let ServerDataObj = {pillars:";
	app+=zoteroPillars.dump();
	app+="};</script>";
	resp.set_content(app.c_str(),"text/html");
    }
}

std::shared_ptr<User> GetUserFromCookie(const Request &req)
{
    const char *ptr = get_header_value(req.headers,"Cookie");
    if(!ptr) return nullptr;
    std::string x = ptr;
    auto pos = x.find("SESSID=");
    auto pos2 = x.find(";",pos);
    std::string cookie = "";

    if(pos2==std::string::npos)
	cookie = x.substr(pos+7);
    else
	cookie = x.substr(pos+7,pos2);

    return UserHandler::GetUserTable().GetUserBySessid(cookie);
}

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void do_authentification(const Request& req, Response &resp)
{
    std::cout<<req.path<<std::endl;

    std::regex reg_meta("/books/[a-zA-Z0-9]+/?$");
    std::regex reg_metajs("/books/[a-zA-Z0-9]+/info.json$");
    std::regex reg_pages("/books/[a-zA-Z0-9]+/pages/index.html$");
    std::regex reg_pages2("/books/[a-zA-Z0-9]+/pages/?$");
    
    int accreq = 1;
    resp.status = 200;
    if(req.path.find("/admin/")!=std::string::npos)
	accreq = 4;
    else if(req.path.find("/write/")!=std::string::npos)
	accreq = 2;
    else if(req.path.find("/backups")!=std::string::npos && req.path.find("/books/")!=std::string::npos)
	accreq = 2;
    else if(std::regex_search(req.path,reg_meta)
	    || std::regex_search(req.path,reg_metajs))
	accreq = 0;

    
    if(!User::AccessCheck(GetUserFromCookie(req),accreq))
    {
	if(std::regex_search(req.path,reg_pages)
	    ||std::regex_search(req.path,reg_pages2))
	{
	    resp.status = 206;
	}
	else
	   resp.status = 403;
	return;
    }

    std::cout<<"Access granted!"<<std::endl;
}

void do_upload(const Request& req, Response &resp, CBookManager &manager)
{
    if(!User::AccessCheck(GetUserFromCookie(req),2))
    {
	resp.set_content("missing_access_rights","text/plain");
	resp.status=403;
	return;
    }

    bool forcedWrite=false;
    std::string writePath = "web/books/";
    std::string scanId = "";
    std::string fileName = "";
    std::string ocr_create = "";
    std::string ocr_lang = "";

    try{forcedWrite = req.get_param_value("forced")=="true";}catch(...){};
    try{scanId = req.get_param_value("scanid");}catch(...){};
    try{fileName = req.get_param_value("filename");}catch(...){};
    try{ocr_create = req.get_param_value("create_ocr");}catch(...){};
    try{ocr_lang = req.get_param_value("language");}catch(...){};

    std::cout<<"Authorized request to upload files"<<std::endl;
    std::cout<<"Parsed fileName: "<<fileName<<std::endl;
    std::cout<<"Parsed scanID: "<<scanId<<std::endl;
    std::cout<<"Parsed force Overwrite: "<<forcedWrite<<std::endl;
    if(scanId==""||fileName==""||fileName.find("..")!=std::string::npos||fileName.find("~")!=std::string::npos||scanId.find("..")!=std::string::npos||scanId.find("~")!=std::string::npos)
    {
	resp.status=403;
	resp.set_content("malformed_parameters","text/plain");
	return;
    }
    auto pos = fileName.find_last_of('.');
    std::string fileEnd = fileName.substr(pos+1,std::string::npos);
    std::string fileNameWithoutEnding = fileName.substr(0,pos);


    if(fileEnd == "txt" || fileEnd == "TXT")
	fileName = "ocr.txt";

    writePath+=scanId;
    std::string directory = writePath;

    writePath+="/";
    writePath+=fileName;

    CBook *book;
    try
    {
	book = manager.getMapOfBooks().at(scanId);
    }
    catch(...)
    {
	resp.status=403;
	resp.set_content("book_unknown","text/plain");
	return;
    }

    bool doOverwrite = std::filesystem::exists(writePath);
    doOverwrite |= std::filesystem::exists(directory+"/pages/"+fileName);
    if((doOverwrite||!std::filesystem::exists(directory))&&!forcedWrite)
    {
	resp.status=403;
	resp.set_content("file_exists","text/plain");
	return;
    }
    const char *buffer = req.body.c_str();
    auto buffer_length = req.body.length();
    char imgbuffer[4*1014*1024];
    if(fileEnd=="jpg"||fileEnd=="png"||fileEnd=="JPG"||fileEnd=="PNG"||fileEnd=="jp2"||fileEnd=="JP2")
    {
	try
	{
	    static std::mutex m;
	    std::lock_guard lck(m);
	    if(fileEnd=="JP2" || fileEnd == "jp2")
	    {
		fileName = fileNameWithoutEnding+".jpg";
		std::string endTmpFileName = "tmp021.jpg";
		bool deleteJP2 = true;
		bool deleteBMP = true;

		std::ofstream ofs("tmp021.jp2",std::ios::out);
		ofs.write(req.body.c_str(),req.body.length());
		ofs.close();

		if(system("opj_decompress -i tmp021.jp2 -o tmp021.bmp") != 0)
		{
		    resp.status = 403;
		    resp.set_content("Could not convert jp2 to jpg missing openjpeg library!","text/plain");
		    return;
		}

		if(system("convert tmp021.bmp tmp021.jpg") == 0)
		{
		}
		
		if((deleteJP2 && (system("rm tmp021.jp2") != 0)) || (deleteBMP && (system("rm tmp021.bmp") != 0)))
		{
		    resp.status = 403;
		    resp.set_content("Cleanup error!","text/plain");
		    return;
		}
		std::ifstream ifs(endTmpFileName.c_str(),std::ios::in);
		if(!ifs)
		{
		    resp.status = 403;
		    resp.set_content("Could not convert jp2 to jpg!","text/plain");
		    return;
		}
		ifs.read(imgbuffer,4*1024*1024);
		auto len = ifs.gcount();
		ifs.close();
		std::string delname = "rm "+endTmpFileName;
		if(system(delname.c_str()) != 0)
		{
		    resp.status = 403;
		    resp.set_content("Cleanup error!","text/plain");
		    return;
		}

		buffer = imgbuffer;
		buffer_length = len;
	    }
	    std::string pa = directory;
	    pa+="/readerInfo.json";
	    nlohmann::json file_desc;
	    if(std::filesystem::exists(pa))
	    {
		std::ifstream readerFile(pa.c_str(),std::ios::in);
		readerFile>>file_desc;
		readerFile.close();
	    }
	    else
	    {
		file_desc["maxPageNum"] = 0;
		file_desc["pages"] = nlohmann::json::array();
	    }

	    nlohmann::json entry;
	    entry["file"] = fileName;
	    std::regex reg("_0*([0-9]+)");
	    std::smatch cm;
	    int maxPageNum = file_desc["maxPageNum"];
	    std::cout<<fileName<<std::endl;
	    std::regex_search(fileName,cm,reg);
	    std::cout<<"Match size: "<<cm.size()<<std::endl;
	    if(cm.size()<2)
	    {
		resp.status = 403;
		resp.set_content("Wrong format: Expected [ bsbid_pagenumber.jpg ]","text/plain");
		return;
	    }
	    std::cout<<"Match: "<<cm[1]<<std::endl;
	    entry["pageNumber"]=std::stoi(cm[1]);
	    maxPageNum = std::max(maxPageNum,std::stoi(cm[1]));

	    int width,height,z;
	    stbi_info_from_memory(reinterpret_cast<const unsigned char*>(buffer),buffer_length,&width,&height,&z);
	    entry["width"] = width;
	    entry["height"] = height;
	    bool replace = false;
	    for(auto &iter : file_desc["pages"])
	    {
		if(iter["pageNumber"]==entry["pageNumber"])
		{
		    iter["width"] = width;
		    iter["height"] = height;
		    iter["file"] = fileName;
		    replace=true;
		    break;
		}
	    }
	    file_desc["maxPageNum"] = maxPageNum;
	    int what = entry["pageNumber"];
	    if(!replace)
		file_desc["pages"].push_back(std::move(entry));

	    std::ofstream writer(pa.c_str(),std::ios::out);
	    writer<<file_desc;
	    writer.close();

	    if(ocr_create=="true")
	    {
		std::cout<<"Creating ocr for image!"<<std::endl;
		book->addPage(ocr_reader.CreateOcrFromImage(reinterpret_cast<const unsigned char*>(buffer),buffer_length,ocr_lang.c_str()),std::to_string(what),std::to_string(maxPageNum));
		std::cout<<"Finished creating ocr!"<<std::endl;
	    }
	}
	catch(std::exception &e)
	{
	    resp.status = 403;
	    std::string error = "Error while creating readerInfo.json: ";
	    error+=e.what();
	    resp.set_content(error.c_str(),"text/plain");
	    return;
	}
	writePath = directory;
	writePath += "/pages";
	std::error_code ec;
	if(!std::filesystem::exists(writePath))
	    std::filesystem::create_directory(writePath,ec);
	writePath+="/";
	writePath += fileName;
    }
    else if((fileEnd!="txt")&&(fileEnd!="TXT"))
    {
	resp.status = 403;
	resp.set_content("Unsupported file type, the uploader only supports .txt, .jpg and .png files!","text/plain");
	return;
    }
    else
    {
	writePath = directory;
	writePath += "/ocr.txt";
	std::cout<<writePath<<std::endl;
    }
    std::cout<<"Uploading file now! File size: "<<req.body.length()<<std::endl;

    if(doOverwrite)
    {
	static std::mutex ml;
	std::lock_guard lck(ml);
	std::string backupfolder = "web/books/";
	backupfolder += scanId;
	backupfolder+="/backups";
	if(!std::filesystem::exists(backupfolder))
	    std::filesystem::create_directory(backupfolder);

	std::string newpath = backupfolder;
	newpath+="/";
	newpath+=fileName;
	if(!std::filesystem::exists(newpath))
	    std::filesystem::create_directory(newpath);

	int version = 0;
	for(const auto &dirEntry : std::filesystem::directory_iterator(newpath))
	{
	    (void)dirEntry;
	    ++version;
	}

	std::string finnewpath = newpath;
	finnewpath+="/";

	std::stringstream ss;
	ss << std::setw(6) << std::setfill('0') << version;
	finnewpath+=ss.str();
	finnewpath+=GetUserFromCookie(req)->GetEmail();
	finnewpath+="-";
	finnewpath+=fileName;
	std::filesystem::rename(writePath,finnewpath);
    }
    std::ofstream ofs(writePath.c_str(),std::ios::out);
    ofs.write(buffer,buffer_length);
    ofs.close();
    resp.status=200;
}

void do_senduserlist(const Request &req, Response &resp)
{
    //Check if the user has admin access
    if(!User::AccessCheck(GetUserFromCookie(req),AccessRights::USR_ADMIN))	
    {
	resp.status = 403;
	return;
    }
    resp.set_content(std::move(UserHandler::GetUserTable().toJSON()),"application/json");
}

void do_usertableupdate(const Request &req, Response &resp)
{
    //Check if the user has admin access
    if(!User::AccessCheck(GetUserFromCookie(req),AccessRights::USR_ADMIN))	
    {
	resp.status = 403;
	return;
    }

    try
    {
	//Try to parse the command list from the body received by zotero
	nlohmann::json js = nlohmann::json::parse(req.body);

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
	resp.status = 200;
    }
    catch(...)
    {
	resp.status = 400;
    }

}

void do_restart_server(const Request &req, Response &resp)
{
    if(!User::AccessCheck(GetUserFromCookie(req),AccessRights::USR_WRITE))	
    {
	resp.status = 403;
	return;
    }


    std::unique_lock<std::mutex> lock(gGlobalUpdateOperationLock, std::try_to_lock);
    if(!lock.owns_lock()){
	//mutex not locked...
	resp.status = 500;
	if(gGlobalUpdateOperation == GlobalUpdateOperations::update_zotero)
	    resp.set_content("updateing_zotero_atm","text/plain");
	else if(gGlobalUpdateOperation == GlobalUpdateOperations::restart_server)
	    resp.set_content("restarting_server_atm","text/plain");
	else
	    resp.set_content("unkown_server_error","text/plain");
	return;
    }
    gGlobalUpdateOperation = GlobalUpdateOperations::restart_server;
    resp.status = 200;
    std::cout<<"User: "<<GetUserFromCookie(req)->GetEmail()<<" issued restart server!"<<std::endl;
    srv.stop();
}

void do_update_zotero(const Request &req, Response &resp)
{
    std::unique_lock<std::mutex> lock(gGlobalUpdateOperationLock, std::try_to_lock);
    if(!lock.owns_lock()){
	//mutex not locked...
	resp.status = 500;
	if(gGlobalUpdateOperation == GlobalUpdateOperations::update_zotero)
	    resp.set_content("updateing_zotero_already","text/plain");
	else if(gGlobalUpdateOperation == GlobalUpdateOperations::restart_server)
	    resp.set_content("restarting_server_atm","text/plain");
	else
	    resp.set_content("unkown_server_error","text/plain");
	return;
    }
    gGlobalUpdateOperation = GlobalUpdateOperations::update_zotero;

    try
    {
	//Check if the user has write access
	if(!User::AccessCheck(GetUserFromCookie(req),AccessRights::USR_WRITE))	
	{
	    resp.status = 403;
	    return;
	}
	std::cout<<"User: "<<GetUserFromCookie(req)->GetEmail()<<" issued update zotero!"<<std::endl;

	//Update the pillars first
	nlohmann::json zot = Zotero::GetPillars();
	std::ofstream of("web/pillars.json",std::ios::out);
	of<<zot;
	of.close();

	nlohmann::json metaData;
	for(auto &it : zot)
	{
	    auto entryjs = nlohmann::json::parse(Zotero::SendRequest(Zotero::Request::GetAllItemsFromCollection(it["key"])));
	    for(auto &it : entryjs)
		metaData.push_back(it);
	}
	//Save the collected data as it is the newest data available
	std::ofstream o("bin/zotero.json",std::ios::out);
	o<<metaData;
	o.close();
	resp.status = 200;
    }
    catch(...)
    {
	resp.set_content("unkown_server_error","text/plain");
	resp.status = 500;
    }
}


int main(int argc, char **argv)
{
    if(argc < 2)
	return 0;
    int startPort = std::stoi(argv[1]);
    std::cout<<"Starting on port: "<<startPort<<std::endl;

    //Register for sigterm as it is send by systemd to stop the service.
    signal(SIGTERM, sig_handler);
    CBookManager manager;
    StaticWebpageCreator creator;
    creator.createSearchPage();
    creator.createInformationPage();
    creator.createAdminPage();
    creator.createUploadPage();

    //Load all pillars
    nlohmann::json zoteroPillars;
    try
    {
	zoteroPillars = Zotero::GetPillars();
	std::ofstream write_pillars("web/pillars.json",std::ios::out);
	if(write_pillars.is_open())
	    write_pillars<<zoteroPillars;
	write_pillars.close();
    }
    catch(...)
    {
	std::cout<<"Careful! You are running in offline mode now! Using last stable pillars for startup."<<std::endl;
	zoteroPillars = nlohmann::json::parse("[{\"key\":\"XCFFDRQC\",\"name\":\"Forschung CLAS\"},{\"key\":\"RFWJC42V\",\"name\":\"Geschichte des Tierwissens\"}]");
	std::cout<<"Offline pillars used: "<<zoteroPillars.dump()<<std::endl;
    }
    nlohmann::json metaData;
    //If there are already metadata dont pull them again
    std::ifstream meta("bin/zotero.json",std::ios::in);
    if(!meta)
    {
	//For every collection in zotero pull all items from zotero and put them into one json
	for(auto &it : zoteroPillars)
	{
	    auto entryjs = nlohmann::json::parse(Zotero::SendRequest(Zotero::Request::GetAllItemsFromCollection(it["key"])));
	    unsigned long numEntries = 0;
	    for(auto &it : entryjs)
	    {
		metaData.push_back(it);
		numEntries++;
	    }
	    it["numEntries"] = numEntries;
	}
	//Save the collected data as it is the newest data available
	std::ofstream o("bin/zotero.json",std::ios::out);
	o<<metaData;
	o.close();
    }
    else
    {
	//Ok we already got metadata so just load it
	meta>>metaData;
    }
    meta.close();
    std::cout<<"Start CBookmanager::UpdateZotero()"<<std::endl;
    for(auto &entry : metaData)
    {
	    if(entry["data"]["key"].get<std::string>() == "QTYP6NYV")
		    std::cout<<"Found everything!!!!!"<<std::endl;
    }
    manager.updateZotero(metaData);
    std::cout<<"Finished CBookmanager::UpdateZotero()"<<std::endl;

    if(manager.initialize())
	std::cout<<"CBookmanager::initialize() finished and successfull!"<<std::endl;
    else
    {	std::cout<<"ERROR in CBookmanager::initialize()"<<std::endl; return 0;}



    //Load the only dynamic page in the server the search.html
    std::ifstream srchFile("web/Search.html",std::ios::in);
    std::string fileSearchHtml;
    if(!srchFile)
	std::cout<<"Major error could not find Search.html"<<std::endl;
    else
    {
	//read the whole file in one go
	fileSearchHtml = std::string((std::istreambuf_iterator<char>(srchFile)), std::istreambuf_iterator<char>());
    }
    srchFile.close();

    nlohmann::json statistics;
    unsigned long ocrcount = 0;
    unsigned long pagecount = 0;
    unsigned long books300 = 0;
    unsigned long books500 = 0;
    unsigned long books1000 = 0;
    unsigned long books1000plus = 0;

    for(auto &it : zoteroPillars)
    {
	    unsigned long numEntries = 0;
	    for(auto it2 : manager.getMapOfBooks())
	    {
		auto vec = it2.second->getMetadata().getCollections();
		if(std::find(vec.begin(),vec.end(),it["key"].get<std::string>()) != vec.end())
			numEntries++;
	    }
	    it["numEntries"] = numEntries;
    }

    for(auto it : manager.getMapOfBooks())
    {
	if(it.second->hasOcr())
	{
	    ++ocrcount;
	    unsigned long cnt = 0;
	    std::filesystem::path p = std::filesystem::path(it.second->getPath())/"pages";
	    if(std::filesystem::exists(p))
	    {
	    for(auto &it2 : std::filesystem::directory_iterator(p))
	    {
		if(it2.path().extension() == ".jpg" || it2.path().extension() == ".png" || it2.path().extension() == ".bmp")
		{
		    ++pagecount;
		    ++cnt;
		}
	    }
	    }
	    
	    if(cnt != 0)
	    {
		if(cnt<300)
		    books300++;
		else if(cnt < 500)
		    books500++;
		else if(cnt < 1000)
		    books1000++;
		else
		    books1000plus++;
	    }
	}
    }
    

    statistics["BookOcrCount"] = ocrcount;
    statistics["BookPageCount"] = pagecount;
    statistics["BookMetadataCount"] = manager.getMapOfBooks().size();
    statistics["Books300"] = books300;
    statistics["Books500"] = books500;
    statistics["Books1000"] = books1000;
    statistics["Books1000plus"] = books1000plus;
    statistics["collections"] = nlohmann::json::array();

    for(auto &it : zoteroPillars)
    {
    	nlohmann::json jk;
    	jk["name"] = it["name"];
    	jk["size"] = it["numEntries"];
    	statistics["collections"].push_back(jk);
    }
    
    inja::Environment env;
    inja::Template temp = env.parse_template("web/statistics/template.html");
    std::string result = env.render(temp,statistics);
    atomic_write_file("web/statistics/index.html",result);


    srv.Post("/login",&do_login);
    srv.Get("/search",[&](const Request &req, Response &resp) { do_search(req,resp,fileSearchHtml,zoteroPillars,manager);});
    srv.Get("/searchinbook",[&](const Request &req, Response &resp) { do_searchinbook(req,resp,manager);});
    srv.Get("/createbibliography",[&](const Request &req, Response &resp) { do_createbiblio(req,resp,manager);});
    srv.Get("/api/v1/typeahead/corpus",[&](const Request &req, Response &resp) { get_sugg(req,resp,manager, "corpus");});
    srv.Get("/api/v1/typeahead/author",[&](const Request &req, Response &resp) { get_sugg(req,resp,manager,"author");});
    srv.Get("/api/v1/shutdown",&do_restart_server);
    srv.Get("/api/v1/update_zotero",&do_update_zotero);
    srv.Get("/getmetadata", [&](const Request &req, Response &resp) { get_metadata(req,resp,manager);});
    srv.Get(R"(/authenticate.*)",&do_authentification);
    srv.Get("/userlist",&do_senduserlist);
    srv.Post("/userlist",&do_usertableupdate);
    srv.Post("/upload",[&](const Request &req, Response &resp){do_upload(req,resp,manager);});
    std::cout<<"C++ Api server startup successfull!"<<std::endl;

    std::thread restart_thread(&update_and_restart);
    restart_thread.detach();

    std::thread update_files(&update_all_files,&manager,&zoteroPillars);
    srv.listen("localhost", startPort);
    update_files.join();
    if(gGlobalUpdateOperation == GlobalUpdateOperations::restart_server)
	return -1;
    return 0;
}



