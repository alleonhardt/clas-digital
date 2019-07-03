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
DEFINE_int32(threads, 0, "Number of threads to listen on. Numbers <= 0 "
		"will use the number of cores on this machine.");

int main(int argc, char* argv[]) {

	alx::cout<<"Starting server..."<<alx::endl;
	
	auto start = std::chrono::system_clock::now();
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

	alx::cout.write("Loading ssl context... ",alx::console::green_black,"done.");


	if (FLAGS_threads <= 0) {
		FLAGS_threads = sysconf(_SC_NPROCESSORS_ONLN);
		CHECK_GT(FLAGS_threads, 0);
		alx::cout.write("\nDetecting processor cores... ", alx::console::green_black,"done.");
		alx::cout.write("\nDetected ",alx::console::yellow_black,(int)FLAGS_threads,alx::console::white_black," processor cores!\n");
//		if(FLAGS_threads==1)
//			FLAGS_threads = 40;
		alx::cout.write(alx::console::yellow_black,"\nContinuing with ",(int)FLAGS_threads,alx::console::white_black," processor threads!\n");
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
	alx::cout.write("Binding server ports... ",alx::console::green_black,"done.\n");

	// Start HTTPServer mainloop in a separate thread
	std::thread t2([&] () {
			for(;;)
			{
				std::string x = std::move(alx::cout.getCommand());
				if(x=="shutdown")
				{
					std::raise(SIGINT);
					debug::gGlobalShutdown = true;
					break;
				}
				HandlerFactory::parseCommands(std::move(x));
			}
	});


	std::thread t([&] () {
			CBookManager &manager = GetSearchHandler::GetBookManager();
				
				nlohmann::json js;
				try
				{
					alx::cout.write("Loading zotero json from disk... ");
					std::ifstream isf("bin/zotero.json",std::ios::in);
					isf>>js;
					alx::cout.write(alx::console::green_black,"done.\n");
				}
				catch(...)
				{
					alx::cout.write(alx::console::yellow_black,"\nNo metadata found![Fetching from server...] ");
					std::ofstream wr("bin/zotero.json",std::ios::out);
					std::string jstr = std::move(Zotero::SendRequest(Zotero::Request::GetAllItems));
					wr<<jstr;
					wr.close();
					js = std::move(nlohmann::json::parse(jstr));
					alx::cout.write(alx::console::green_black,"done.\n");
				}
				try
				{
					alx::cout.write("Fetching zotero metadata in book manager... ");
					manager.updateZotero(js);
				}
				catch(...)
				{
					alx::cout.write(alx::console::red_black,"failed.\n");
					alx::cout.writeln(DBG_HEADER,"[Could not initialise book manager!]");
					sleep(3);
					return;
				}
				alx::cout.write(alx::console::green_black,"done.\n");
				Zotero::GetPillars();

			if(manager.initialize())
				alx::cout.write("CBookManager initialisation: ",alx::console::green_black,"SUCCESS\n");
			else
				alx::cout.write("CBookManager initialisation: ",alx::console::red_black,"FAILURE\n");

			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end-start;
			alx::cout.writeln(DBG_HEADER,"[Minor error please ignore!]");
			alx::cout.write(alx::console::yellow_black,"Server startup took ",elapsed_seconds.count(),"s\n");
			server.start();
	});

	t.join();
	t2.join();
	return 0;
}
