#include <iostream>
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include "httplib.h"

using namespace httplib;

int main()
{
	std::cout<<"Stuff works kind of nice!"<<std::endl;
	Server srv;
	srv.Get("/api",[](const Request &, Response &resp) {
			resp.set_content("Hello World from new c server!", "text/plain");
			});
	srv.listen("localhost", 9000);
	return 0;
}
