#ifdef COMPILE_UNITTEST 
#include <gtest/gtest.h>
#endif

#include <folly/Memory.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/Unistd.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <list>
#include <string>
#include <csignal>
#include <experimental/filesystem>
#include "src/books/CBookManager.hpp"
#include "src/books/func.hpp"
#include "src/server/HandlerFactory.hpp"
#include "src/zotero/zotero.hpp"
#include "src/console/console.hpp"

using namespace proxygen;

using folly::EventBase;
using folly::EventBaseManager;
using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;

DEFINE_int32(http_red,1408,"Port to redirect HTTP to HTTPSS");
DEFINE_int32(http_port, 1409, "Port to listen on with HTTP protocol");
DEFINE_int32(h2_port, 11002, "Port to listen on with HTTP/2 protocol");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_int32(threads, 2, "Number of threads to listen on. Numbers <= 0 "
		"will use the number of cores on this machine.");
#ifndef COMPILE_UNITTEST

int main(int argc, char* argv[]) {

	alx::cout<<"Starting server..."<<alx::endl;
	folly::init(&argc, &argv, true);


	wangle::SSLContextConfig cfg;
	cfg.setCertificate("bin/cert.pem","bin/key.pem","");
	cfg.isDefault = true;
	std::vector<HTTPServer::IPConfig> IPs = {
		{SocketAddress(0, FLAGS_http_red, true), Protocol::HTTP},
		{SocketAddress(0, FLAGS_http_port, true), Protocol::HTTP},
		{SocketAddress(0, FLAGS_h2_port, true), Protocol::HTTP2}
	};
	IPs[1].sslConfigs = std::vector<wangle::SSLContextConfig>{cfg};
	IPs[2].sslConfigs = std::vector<wangle::SSLContextConfig>{cfg};



	if (FLAGS_threads <= 0) {
		FLAGS_threads = sysconf(_SC_NPROCESSORS_ONLN);
		CHECK_GT(FLAGS_threads, 0);
	}
	HTTPServerOptions options;
	options.threads = static_cast<size_t>(FLAGS_threads);
	options.idleTimeout = std::chrono::milliseconds(60000);
	options.shutdownOn = {SIGINT, SIGTERM};
	options.enableContentCompression = false;
	options.handlerFactories = RequestHandlerChain()
		.addThen<HandlerFactory>()
		.build();
	options.h2cEnabled = true;

	auto diskIOThreadPool = std::make_shared<folly::CPUThreadPoolExecutor>(
			FLAGS_threads,
			std::make_shared<folly::NamedThreadFactory>("StaticDiskIOThread"));
	folly::setCPUExecutor(diskIOThreadPool);

	HTTPServer server(std::move(options));
	server.bind(IPs);

	// Start HTTPServer mainloop in a separate thread
	std::thread t2([&] () {
			for(;;)
			{
				std::string x = std::move(alx::cout.getCommand());
				if(x=="quit")
				{
					std::raise(SIGINT);
					debug::gGlobalShutdown = true;
					break;
				}
				HandlerFactory::parseCommands(std::move(x));
			}
	});


	std::thread t([&] () {
				try
				{
					nlohmann::json js;
					std::ifstream isf("bin/zotero.json",std::ios::in);
					isf>>js;
				}
				catch(...)
				{
					Zotero zot;
					std::ofstream wr("bin/zotero.json",std::ios::out);
					std::string js = std::move(zot.SendRequest(Zotero::Request::GetAllItems));
					wr<<js;
					wr.close();
				}
			
			alx::cout<<"Read zotero files with success!"<<alx::endl;

			server.start();
	});

	t.join();
	t2.join();
	return 0;
}
#else

int main(int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#endif
