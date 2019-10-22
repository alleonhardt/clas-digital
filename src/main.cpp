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


int main()
{
	//Register for sigterm as it is send by systemd to stop the service.
	signal(SIGTERM, sig_handler);

	auto zoteroPillars = Zotero::GetPillars();
	nlohmann::json metaData;
	std::ifstream meta("bin/zotero.json",std::ios::in);
	if(!meta)
	{
		for(auto &it : zoteroPillars)
		{
			auto entryjs = nlohmann::json::parse(Zotero::SendRequest(Zotero::Request::GetAllItemsFromCollection(it["key"])));
			for(auto &it : entryjs)
				metaData.push_back(it);
		}
		std::ofstream o("bin/zotero.json",std::ios::out);
		o<<metaData;
		o.close();
	}
	else
	{
		meta>>metaData;
	}
	meta.close();

	std::ifstream srchFile("web/Search.html",std::ios::in);
	std::string fileSearchHtml;
	if(!srchFile)
		std::cout<<"Major error could not find Search.html"<<std::endl;
	else
	{
		fileSearchHtml = std::string((std::istreambuf_iterator<char>(srchFile)), std::istreambuf_iterator<char>());
	}
	srchFile.close();


	srv.Get("/api",[](const Request &req, Response &resp) {
			std::cout<<"Request to: "<<req.path<<std::endl;
			for(auto &it : req.headers)
			{
			std::cout<<it.first<<": "<<it.second<<std::endl;
			}
			resp.set_content("Hello World from new c server!", "text/plain");
			});

	srv.Post("/login",[](const Request &req, Response &resp) {
			URLParser parser(req.body);
			auto &usrtable = UserHandler::GetUserTable();
			auto ret = usrtable.DoLogin(parser["email"],parser["password"]);
			std::cout<<"Cookie created: "<<ret<<std::endl;
			if(ret!="")
			{
			resp.set_content("<html><head><script>window.location='/';</script></head><body></body></html>","text/html");
			std::string cookie = "SESSID="+ret;
			cookie+="; SECURE";
			resp.set_header("Set-Cookie",cookie.c_str());
			}
			else
			{
			resp.set_content("<html><head></head><body><h1>Access Denied 403</body></html>","text/html");
			}
			});
	srv.Get("/search",[&](const Request &req, Response &resp) {
			std::cout<<"Request to search to : "<<req.uri<<std::endl;
			std::string app = fileSearchHtml;
			app+="<script>let ServerDataObj = {pillars:";
			app+=zoteroPillars.dump();
			app+="};</script>";
			resp.set_content(app.c_str(),"text/html");
			});
	srv.Get("/authenticate",[](const Request &req, Response &resp) {
			
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
				
			});
	std::cout<<"C++ Api server startup successfull!"<<std::endl;
	srv.listen("localhost", 9000);
	return 0;
}

