/**
 * @file src/server/PostHandler.cpp
 * @brief Implements the interface for the PostHandler class and handles all user logins
 *
 *
 */
#include "BasicHandlers.hpp"
#include <proxygen/lib/utils/ParseURL.h>

using namespace proxygen;

PostHandler::PostHandler(User *from) : EmptyHandler(from)
{}

void PostHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	std::cout<<"Received Post Request!"<<std::endl;
	return;
}

void PostHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept
{
	std::cout<<"Received post request with body" <<std::endl;
	SendAccessDenied(downstream_);
	return;
}
