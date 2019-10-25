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

using namespace httplib;

//Defining the srv global to stop the execution once the SIGTERM signal is caught.
Server srv;

void sig_handler(int)
{
    //Just stop the server when systemd wants to exit us.
    srv.stop();
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


	mapPtr = std::unique_ptr<std::map<int,std::vector<std::string>>>(book->getPages(std::move(query),Fuzzyness));

	auto glambda = [](std::string const &str, std::string const &from,std::string const &to) -> std::string {return std::regex_replace(str,std::regex(from),to);};
	for(auto const &it : *mapPtr)
	{
	    if(Fuzzyness==0)
	    {
		int k = it.first;
		js["books"].push_back(k);
	    }
	    else
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
	resp.set_content(js.dump(),"application/json");
    }
    catch(...)
    {
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
	    //It is a search query!
	    std::string query = req.get_param_value("q",0);
	    int fuzz = std::stoi(req.get_param_value("fuzzyness",0));
	    bool auth_only = req.get_param_value("title_only",0)=="true";
	    bool ocr_only = req.get_param_value("ocr_only",0) == "true";
	    std::string author = req.get_param_value("author",0);
	    int pubafter = std::stoi(req.get_param_value("publicatedafter"));
	    int pubbef = std::stoi(req.get_param_value("publicatedbefore"));
	    bool sortway = req.get_param_value("sortway",0) == "sortbyrelevance";
	    std::string pill = req.get_param_value("pillars",0);
	    int page = std::stoi(req.get_param_value("page"));

	    std::vector<std::string> pillars;


	    own_split(pill,',',pillars);

	    std::cout<<"Receveived search request!"<<std::endl;
	    std::cout<<"Query: "<<query<<"; fuzz: "<<fuzz<<"; title only: "<<auth_only<<"; ocr only: "<<ocr_only<<"; author: "<<author<<"; publicated after: "<<pubafter<<"; publicated before: "<<pubbef<<"; sorting by relevance: "<<sortway<<"; searched pillars: "<<pill<<"; vector pillar size: "<<pillars.size()<<std::endl;


	    CSearchOptions options(query,fuzz,pillars,auth_only,ocr_only,author,pubafter,pubbef,true,sortway);
	    std::cout<<"Starting search!"<<std::endl;

	    auto bklst = manager.search(&options);
	    std::cout<<"Finished searching constructing response json"<<std::endl;
	    nlohmann::json js;

	    auto start = std::chrono::system_clock::now();
	    auto &map = manager.getMapOfBooks();
	    auto iter = bklst->begin();
	    if(bklst->size()<(static_cast<unsigned int>(page-1)*10))
		throw 0;
	    std::advance(iter,(page-1)*10);
	    int counter = 0;
	    for(;iter!=bklst->end();++iter)
	    {
		auto &it = map.at(*iter);
		nlohmann::json entry;
		entry["scanId"] = it->getKey();
		entry["copyright"] = !it->getPublic();
		entry["hasocr"] = it->getOcr();
		entry["description"] = it->getMetadata().getShow();
		entry["bibliography"] = it->getMetadata().getMetadata("bib");
		entry["preview"] = it->getPreview(query);
		js["books"].push_back(std::move(entry));
		if(++counter==10)
		    break;
	    } 

	    js["max_results"] = bklst->size();
	    js["page"] = page;
	    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
	    js["time"] = elapsed_seconds.count();
	    std::cout<<"Finished constructing json response. Constructing html response!"<<std::endl;
	    std::string app = fileSearchHtml;
	    app+="<script>let ServerDataObj = {pillars:";
	    app+=zoteroPillars.dump();
	    app+=", search:";
	    app+=js.dump();
	    app+="};</script>";
	    resp.set_content(app.c_str(),"text/html");
	}
	catch(...)
	{
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

void do_authentification(const Request& req, Response &resp)
{

    int accreq = 1;
    if(req.path.find("/admin/")!=std::string::npos)
	accreq = 4;
    else if(req.path.find("/write/")!=std::string::npos)
	accreq = 2;

    resp.status = 403;

    if(User::AccessCheck(GetUserFromCookie(req),accreq))
	resp.status = 200;
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


int main()
{
    //Register for sigterm as it is send by systemd to stop the service.
    signal(SIGTERM, sig_handler);
    CBookManager manager;

    //Load all pillars
    nlohmann::json zoteroPillars;
    try
    {
	zoteroPillars = Zotero::GetPillars();
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
	    for(auto &it : entryjs)
		metaData.push_back(it);
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



    srv.Post("/login",&do_login);
    srv.Get("/search",[&](const Request &req, Response &resp) { do_search(req,resp,fileSearchHtml,zoteroPillars,manager);});
    srv.Get("/searchinbook",[&](const Request &req, Response &resp) { do_searchinbook(req,resp,manager);});

    srv.Get("/authenticate",&do_authentification);
    srv.Get("/userlist",&do_senduserlist);
    srv.Post("/userlist",&do_usertableupdate);
    std::cout<<"C++ Api server startup successfull!"<<std::endl;
    srv.listen("localhost", 9000);
    return 0;
}

