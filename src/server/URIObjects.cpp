#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <experimental/filesystem>
#include "src/server/URIObjects.hpp"
using namespace proxygen;
namespace fs = std::experimental::filesystem;

void UserSystemHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	if(!_user)
		return SendErrorNotFound(downstream_);
	if(headers->getPath() == "/getProfileInfo")
	{
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(_user->toJSON()))
			.sendWithEOM();
		return;
	}
	else if(headers->getPath() == "/userList")
	{
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.header("Content-Type","application/json")
			.body(std::move(UserHandler::GetUserTable().toJSON()))
			.sendWithEOM();
		return;
	}
	else return SendErrorNotFound(downstream_);
}




void UpdateUserSystemHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept
{
	try
	{
		nlohmann::json js = nlohmann::json::parse(std::string(reinterpret_cast<const char*>(body->data()),body->length()));
		for(auto &it : js)
		{
			if(it["action"]=="DELETE")
			{
				UserHandler::GetUserTable().RemoveUser(it["email"]);
			}
			else if(it["action"]=="CREATE")
			{
				UserHandler::GetUserTable().AddUser(it["email"],it["password"],it["access"]);
			}
			else if(it["action"]=="CHANGEACCESS")
			{
				UserHandler::GetUserTable().SetAccessRights(it["email"],it["access"]);
			}
		}
		ResponseBuilder(downstream_)
			.status(200,"Ok")
			.sendWithEOM();
	}
	catch(...)
	{
		SendErrorNotFound(downstream_,"<h1>Received corrupted json!</h1>");
	}
}

std::string DirectoryFilesToJSON(const std::string &path)
{
	if (fs::exists(path) && fs::is_directory(path))
	{
		std::string js = "[";
		for(const auto &dirEntry : fs::directory_iterator(path))
		{
			js+="{\"name\":\"";
			js+=dirEntry.path().filename();
			js+="\"},";
		}
		js[js.length()-1]=']';
		return std::move(js);
	}
	return "";
}


void GetBookRessource::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept
{
	if(!_user||_user->GetAccessRights()==0)
		return SendAccessDenied(downstream_);

	try
	{
		std::string book = headers->getDecodedQueryParam("book");
		std::string res = headers->getDecodedQueryParam("ressource");
		if(book==""&&res=="")
		{
			//Return a list of all books with bibliography and ocr yes false
		}
		else if(book!=""&&res=="")
		{
			std::string path = "web/books/";
			path+=book;
			std::string js = std::move(DirectoryFilesToJSON(path));
			if(js=="")
				throw 0;

			ResponseBuilder(downstream_)
				.status(200,"Ok")
				.header("Content-Type","application/json")
				.body(std::move(js))
				.sendWithEOM();
		}
		else if(book!=""&&res!="")
		{
			std::string path = "web/books/";
			path+=book;
			path+="/";
			path+=res;
			//Returns the ressource
			URIFile fl(std::move(path));
			ResponseBuilder(downstream_)
				.status(200,"Ok")
				.header("Content-Type",fl.getMimeType())
				.body(std::move(fl.getBufferReference()))
				.sendWithEOM();
		}
	}
	catch(...)
	{
		return SendErrorNotFound(downstream_);
	}
}
