/**
 * @file src/server/PostHandler.cpp
 * @brief Implements the interface for the PostHandler class and handles all user logins
 *
 *
 */
#include "BasicHandlers.hpp"
#include <proxygen/lib/utils/ParseURL.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

using namespace proxygen;

void PostHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept
{
	HTTPMessage msg;
	std::string url = "/myurl?";
	url+=std::string(reinterpret_cast<const char*>(body->data()),body->length());
	msg.setURL(url);

	try
	{
		std::string sessid = "SESSID=";
		sessid+=UserHandler::GetUserTable().DoLogin(msg.getDecodedQueryParam("email"),msg.getDecodedQueryParam("password"));
		if(sessid=="SESSID=")
			throw 0;
		sessid+="; SECURE";
		URIFile file("web/index.html");
	
		//Sends the response with the created cookie
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Set-Cookie",std::move(sessid))
			.body(std::move(file.getBufferReference()))
			.sendWithEOM();
		return;
	}
	catch(...)
	{
		return SendAccessDenied(downstream_);
	}
	return;
}
