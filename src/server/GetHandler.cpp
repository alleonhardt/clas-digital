#include "GetHandler.hpp"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/FileUtil.h>
#include <folly/executors/GlobalExecutor.h>
#include <ctime>
#include <fstream>
#include <string_view>

using namespace proxygen;

void GetHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
	std::string path = "web/index.html";

	std::unique_ptr<folly::File> _file;
	try
	{
		_file = std::make_unique<folly::File>("web/index.html");
	}
	catch(...)
	{
		ResponseBuilder(downstream_)
			.status(404,"Not found")
			.header("Content-Type","text/html")
			.body("<h1>Not found!</h1>")
			.sendWithEOM();
		return;
	}

	folly::IOBufQueue buf;
	auto data = buf.preallocate(4096,4096);

	int rc = folly::readNoInt(_file->fd(), data.first, data.second);
	buf.postallocate(rc);

	std::string smax = "max-age=";
	std::string type = "text/plain";
	auto pos = path.find_last_of('.');
	if(pos!=std::string::npos)
	{
		std::string_view filEnd = std::string_view(path).substr(pos+1);
		if(filEnd=="js")
			type="application/javascript";
		else if(filEnd=="json")
			type="application/json";
		else if(filEnd=="html")
			type="text/html";
		else if(filEnd=="css")
			type="text/css";
		else if(filEnd=="jpg")
			type="image/jpeg";
		else if(filEnd=="png")
			type="image/png";
		else if(filEnd=="gif")
			type=="image/gif";
	}

	if constexpr(true)
	{
		smax = "no-cache";
	}
	ResponseBuilder(downstream_)
		.status(200, "OK")
		.header("Cache-Control",smax)
		.header("Content-Type",type.c_str())
		.body(buf.move())
		.sendWithEOM();
}

void GetHandler::onEgressPaused() noexcept {
	// This will terminate readFile soon
}

void GetHandler::onEgressResumed() noexcept {
}


void GetHandler::onBody(std::unique_ptr<folly::IOBuf> /*body*/) noexcept {
	// ignore, only support GET
}

void GetHandler::onEOM() noexcept {
}

void GetHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
	// handler doesn't support upgrades
}

void GetHandler::requestComplete() noexcept {
}

void GetHandler::onError(ProxygenError /*err*/) noexcept {
}
