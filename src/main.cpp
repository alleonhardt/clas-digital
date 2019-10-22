#include <iostream>
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include "httplib.h"

using namespace httplib;

int main()
{
	Server srv;
	srv.Get("/api",[](const Request &req, Response &resp) {
			std::cout<<"Request to: "<<req.path<<std::endl;
			for(auto &it : req.headers)
			{
			std::cout<<it.first<<": "<<it.second<<std::endl;
			}
			resp.set_content("Hello World from new c server!", "text/plain");
			});
	
	srv.Get("/login",[](const Request &req, Response &resp) {
			std::cout<<"Received request!"<<std::endl;
			std::cout<<"Request body: "<<req.body<<std::endl;
			resp.set_content("Hello World login","text/plain");
			});
	srv.Get("/authenticate",[](const Request &, Response &resp) {
		resp.status = 403;
	});
	std::cout<<"C++ Api server startup successfull!"<<std::endl;
	srv.listen("localhost", 9000);
	return 0;
}
