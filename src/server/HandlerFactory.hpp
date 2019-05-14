#pragma once
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
#include <map>

#include "server/GetHandler.hpp"

using namespace proxygen;
using folly::EventBase;
using folly::EventBaseManager;
using folly::SocketAddress;



/**
 * This class redirects every request to the right handler, in order to do so it keeps book of every URI object registered
 * and sends the request to the URI object if there is any else the request goes to the default handler.
 */
class HandlerFactory : public RequestHandlerFactory {
    private: 
    public:
        /**
         * Initialises the HandlerFactory and gets called by the proxygen server as the server starts up.
         * Reserve all the post URI Objects as well as all the GET URI Objects.
         */
        void onServerStart(folly::EventBase* /*evb*/) noexcept override 
        {
		}

        void onServerStop() noexcept override {}

        /**
         * As soon as an request is received this function gets called by the proxygen server and expects to return an Request handler.
         * This function does nothing more than to select the correct request handler and return a new instance of him to the proxygen server.
         * @param hdr The header of the message received, is used to set identify the user the message was sent from
         * @return Returns the correct request handler for the requested URI
         */
        RequestHandler* onRequest(RequestHandler*, HTTPMessage* hdr) noexcept override {
			return new GetHandler;
        }
};

