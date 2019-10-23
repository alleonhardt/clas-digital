#include <iostream>
#include <streambuf>
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include "httplib.h"
#include "login/user_system.hpp"
#include "util/URLParser.hpp"
#include "zotero/zotero.hpp"

#include <signal.h>

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



void do_search(const Request& req, Response &resp, const std::string &fileSearchHtml)
{
    if(req.has_param("q"))
    {
	try
	{
	    //This is a search query!
	    std::string query = req.get_param_value("q",0);
	    int fuzz = std::stoi(req.get_param_value("fuzzyness",0));
	    bool auth_only = req.get_param_value("title_only",0)=="true";
	    bool ocr_only = req.get_param_value("ocr_only",0) == "true";
	    std::string author = req.get_param_value("author",0);
	    int pubafter = std::stoi(req.get_param_value("publicatedafter"));
	    int pubbef = std::stoi(req.get_param_value("publicatedbefore"));
	    bool sortway = req.get_param_value("sortway",0) == "sortbyrelevance";
	    std::string pill = req.get_param_value("pillars",0);
	    std::vector<std::string> pillars;
	    auto start = 0;
	    while(true)
	    {
		auto pos = pill.find(",",start);
		std::string tmp;
		if(pos==std::string::npos)
		{
		    tmp = pill.substr(start);
		}
		else
		{
		    tmp = pill.substr(start,pos);
		}
		if(tmp!="")
		    pillars.push_back(std::move(tmp));
		if(pos==std::string::npos)
		    break;
		start = pos+1;
	    }

	    std::cout<<"Receveived search request!"<<std::endl;
	    std::cout<<"Query: "<<query<<"; fuzz: "<<fuzz<<"; author only: "<<auth_only<<"; ocr only: "<<ocr_only<<"; author: "<<author<<"; publicated after: "<<pubafter<<"; publicated before: "<<pubbef<<"; sorting by relevance: "<<sortway<<"; searched pillars: "<<pill<<"; vector pillar size: "<<pillars.size()<<std::endl;

	    std::string app = fileSearchHtml;
	    app+="<script>let ServerDataObj = {pillars:";
	    app+=zoteroPillars.dump();
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

void do_authentification(const Request& req, Response &resp)
{

    int accreq = 1;
    if(req.path.find("/admin/")!=std::string::npos)
	accreq = 4;
    else if(req.path.find("/write/")!=std::string::npos)
	accreq = 2;

    resp.status = 403;
    const char *ptr = get_header_value(req.headers,"Cookie");
    if(!ptr) return;
    std::string x = ptr;
    auto pos = x.find("SESSID=");
    auto pos2 = x.find(";",pos);
    std::string cookie = "";
    if(pos2==std::string::npos)
	cookie = x.substr(pos+7);
    else
	cookie = x.substr(pos+7,pos2);
    std::cout<<"Cookie found: "<<cookie<<std::endl;

    auto &usrtable = UserHandler::GetUserTable();
    auto usr = usrtable.GetUserBySessid(cookie);
    if(usr && ((usr->GetAccessRights()&accreq)==accreq))
	resp.status = 200;
}


int main()
{
    //Register for sigterm as it is send by systemd to stop the service.
    signal(SIGTERM, sig_handler);

    //Load all pillars
    auto zoteroPillars = Zotero::GetPillars();
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



    srv.Post("/login",&login);
    srv.Get("/search",[&](const Request &req, Response &resp) { do_search(req,resp,fileSearchHtml);});

    srv.Get("/authenticate",&do_authentification);
    std::cout<<"C++ Api server startup successfull!"<<std::endl;
    srv.listen("localhost", 9000);
    return 0;
}

