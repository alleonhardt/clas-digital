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

void PostHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	_logout = false;
	if(headers->getPath() == "/logout")
	{
		_logout = true;
		UserHandler::GetUserTable().RemoveSession(headers->getCookie("SESSID").toString());
		URIFile fl("web/guest_index.html");
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","text/html")
			.body(std::move(fl.getBufferReference()))
			.sendWithEOM();
	}
}

void PostHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept
{
	if(!_logout)
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
			sessid+=";SECURE;";
			URIFile file("web/index.html");

			auto _user = UserHandler::GetUserTable().GetUserByName(msg.getDecodedQueryParam("email"));
			nlohmann::json js;
			if(_user)
			{
				nlohmann::json usr;
				usr["email"] = _user->GetEmail();
				usr["access"] = _user->GetAccessRights();
				js["user"] = std::move(usr);
			}

			//Sends the response with the created cookie
			ResponseBuilder(downstream_)
				.status(200,"Ok")
				.header("Set-Cookie",std::move(sessid))
				.body(std::move(file.getBufferReference()))
				.body(URIFile::PrepareDataForSending(std::move(js)))
				.sendWithEOM();
			return;
		}
		catch(...)
		{
			return SendAccessDenied(downstream_);
		}
	}
	return;
}
