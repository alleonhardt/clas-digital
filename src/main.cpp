#include <iostream>
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include "httplib.h"
#include "login/user_system.hpp"
#include "util/URLParser.hpp"


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
