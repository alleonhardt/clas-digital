/**
 * @file src/server/HandlerFactory.hpp
 * 
 * This file contains the basic static handler factory used to instantiate the proxygen server
 * The Handler Factory selects based on the request the appropriate request handler class
 */
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

#include "src/server/BasicHandlers.hpp"
#include "src/server/URIObjects.hpp"

using namespace proxygen;
using folly::EventBase;
using folly::EventBaseManager;
using folly::SocketAddress;


template<typename T>
EmptyHandler *CreateHandler()
{
	return new T();
}

/**
 * @brief The HandlerFactory is used to instantiate the proxygen server and creates all request handler for every request directed to the server
 * This class redirects every request to the right handler, in order to do so it keeps book of every URI object registered
 * and sends the request to the URI object if there is any else the request goes to the default handler.
 */
class HandlerFactory : public RequestHandlerFactory {
    private:
		std::map<std::string,EmptyHandler*(*)()> _getMap; ///< The map of all get handlers used to map every get handler to a specific URI
		std::map<std::string,EmptyHandler*(*)()> _postMap;	///< The map of all post handlers used to map every post handler to a specific URI
    public:
        /**
         * Initialises the HandlerFactory and gets called by the proxygen server as the server starts up.
         * Reserve all the post URI Objects as well as all the GET URI Objects.
         */
        void onServerStart(folly::EventBase* /*evb*/) noexcept override 
		{
			_getMap["/getProfileInfo"] = &CreateHandler<UserSystemHandler>;
			_getMap["/userList"] = &CreateHandler<UserSystemHandler>;
			_getMap["/getBookRessources"] = &CreateHandler<GetBookRessource>;
			_getMap["/searchword"] = &CreateHandler<GetSearchHandler>;

			_postMap["/changeUserTable"] = &CreateHandler<UpdateUserSystemHandler>;
			_postMap["/logout"] = &CreateHandler<PostHandler>;
		}

		/**
		 * @brief Handles the cleanup and behaviour if the server is stopped, in this case it does nothing at all because no cleanup is needed
		 *
		 */
        void onServerStop() noexcept override {}

        /**
         * As soon as an request is received this function gets called by the proxygen server and expects to return an Request handler.
         * This function does nothing more than to select the correct request handler and return a new instance of him to the proxygen server.
         * @param hdr The header of the message received, is used to set identify the user the message was sent from
         * @return Returns the correct request handler for the requested URI
         */
        RequestHandler* onRequest(RequestHandler*, HTTPMessage* hdr) noexcept override 
		{
			alx::cout.write("Request to: ",hdr->getURL(),"\n");
			if(hdr->getMethod() == HTTPMethod::GET)
				return onRequest<GetHandler>(_getMap,hdr);
			return onRequest<PostHandler>(_postMap,hdr);
        }

		/**
		 * @brief This function handles all requests to either getMap or postMap depending on the first parameter
		 * It creates the right RequestHandler and sets the user parameter in the handler class to the user that does the request
		 *
		 * @param mp The map to use for looking up the URI function mapping
		 * @param hdr The http message received by the user
		 *
		 * @return The request handler which is about to handle the received request
		 */
		template<typename T>
		RequestHandler *onRequest(std::map<std::string,EmptyHandler*(*)()> &mp, HTTPMessage *hdr)
		{
			//Start with an empty empty handler
			EmptyHandler *ret = nullptr;
			try
			{
				//Try to get the right request handler if the path is unknown the function will throw an error
				ret = (*mp.at(hdr->getPath()))();
			}
			catch(...)
			{
				//If an error was thrown check if we have to deallocate a dangling pointer if yes delete it
				if(ret)
					delete ret;
				//Create a new default handler which is going to handle the request no one could satify
				ret = new T();
			}
			//Set the user in the empty handler to the user we deduced from the cookie he gave us is going to be nullptr if he does not have a valid session key
			ret->_user = std::move(UserHandler::GetUserTable().GetUserBySessid(hdr->getCookie("SESSID").toString()));
			//Return the request handler
			return ret;
		}

		static void parseCommands(std::string command);
};

