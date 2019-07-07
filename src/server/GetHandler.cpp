/**
 * @file src/server/GetHandler.cpp
 *
 * @brief Implements the interface of the GetHandler class and the interface of the URIFile class
 *
 * Implements the interface of the gethandler class and the default response functions also hosts the file Map which mappes almost all files to a specific URI with the given access rights so one can do access checks on files
 */
#include "BasicHandlers.hpp"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/FileUtil.h>
#include <folly/executors/GlobalExecutor.h>
#include <ctime>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include "src/zotero/zotero.hpp"

using namespace proxygen;

URIFile::URIFile(std::string path, int accessRights) : _path(path),_access(accessRights)
{
	loadBuffer();
	auto pos = path.find_last_of('.');
	_mimeType = "text/plain";
	if(pos!=std::string::npos)
	{
		std::string_view filEnd = std::string_view(path).substr(pos+1);
		if(filEnd=="js")
			_mimeType="application/javascript";
		else if(filEnd=="json")
			_mimeType="application/json";
		else if(filEnd=="html")
			_mimeType="text/html";
		else if(filEnd=="css")
			_mimeType="text/css";
		else if(filEnd=="jpg")
			_mimeType="image/jpeg";
		else if(filEnd=="png")
			_mimeType="image/png";
		else if(filEnd=="gif")
			_mimeType="image/gif";
	}
}

void URIFile::loadBuffer()
{
	//Open the file
	std::ifstream file(_path.c_str(), std::ios::binary);
	if(!file)
		throw 0;

	file.seekg(0,file.end);
	auto fileSize = file.tellg();
	//Create a buffer with the file size as capacity
	_fileBuf = folly::IOBuf::create(fileSize);
	//Clear the eof bit of filestream
	file.clear();
	//go to the beginning of the file again
	file.seekg(0, file.beg);
	//Read the file into the buffer
	file.read(reinterpret_cast<char*>(_fileBuf->writableData()),fileSize);
	//Tell the buffer how much data we put inside the buffer
	_fileBuf->append(fileSize);
}

std::unique_ptr<folly::IOBuf> &URIFile::getBufferReference()
{
	if(!_fileBuf)
		loadBuffer();
	return _fileBuf;
}

std::unique_ptr<folly::IOBuf> URIFile::getBuffer()
{
	if(!_fileBuf)
		loadBuffer();
	//Clones the buffer, makes it shared by facebook definition
	return _fileBuf->clone();
}

std::string URIFile::PrepareDataForSending(nlohmann::json &&js)
{
	std::string xy;
	xy = "<script>let ServerDataObj = ";
	xy+=js.dump();
	xy+="</script>";
	return std::move(xy);
}

bool URIFile::doAccessCheck(int acc) const
{
	//If no access is neeeded return always true
	if(_access==0)
		return true;
	else if((acc&_access)==_access) //Check if the user got the exact same bit set that is needed to access this file
		return true;	
	
	return false;
}

const std::string &URIFile::getPath() const
{
	return _path;
}

const std::string &URIFile::getMimeType() const
{
	return _mimeType;
}

//The map with all managed files, contains a buffer to the file and the needed access rights to access it
std::unordered_map<std::string,URIFile> fileAccess {
	{"/",URIFile("web/guest_index.html",0)},
	{"/favicon.ico",URIFile("web/favicon.png",0)},
	{"/home",URIFile("web/index.html",1)},
	{"/search",URIFile("web/Search.html",0)},
	{"/administration",URIFile("web/Administration.html",4)},
	{"/uploadbook",URIFile("web/UploadBook.html",2)},
	{"/managebooks",URIFile("web/ManageBooks.html",2)},
	{"/GetBooks",URIFile("web/GetBooks.html",0)},
	{"/ShowMetadata", URIFile("web/ShowMetadata.html",0)},
	{"/scan.png",URIFile("web/scan.png",0)},
	{"/404.jpeg",URIFile("web/404.jpeg",0)},
	{"/volltext.png",URIFile("web/volltext.png",0)},
	{"/jszip.js",URIFile("web/jszip.js",2)},
	{"/zotero",URIFile("web/Zotero.html",2)}
}; ///< The map which caches all file the get handler will return and also saves the access rights to acces these files

void ReloadAllFiles()
{
	for(auto &it : fileAccess)
	{
		std::unique_ptr<folly::IOBuf> ptr(std::move(it.second.getBufferReference()));
	}
}


void SendErrorNotFound(proxygen::ResponseHandler *rsp, std::string message)
{
	//Sends an 404 not found response to the client and specify the content
	//as html so the user can design a good looking version of the answer
	ResponseBuilder(rsp)
		.status(404,"Not found")
		.header("Content-Type","text/html")
		.body(message.c_str())
		.sendWithEOM();
}

void SendAccessDenied(proxygen::ResponseHandler *rsp, std::string message)
{
	//Sends an access denied response to the downstream client and format
	//it in html so the user can specify an response in html
	ResponseBuilder(rsp)
		.status(401,"Access denied")
		.header("Content-Type","text/html")
		.body(message.c_str())
		.sendWithEOM();
}



void GetHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
	int access = 0;
	if(_user)
		access = _user->GetAccessRights();
	//As long as we dont have users yet just set the access here
	try
	{
		URIFile *file;
		if(headers->getPath()=="/" && _user)
		{
			file = &fileAccess.at("/home");
		}
		else
		{
			//Have a look if the file requested does exist at all
			file = &fileAccess.at(headers->getPath());
		}

		//Ok the file does exist so do the access check based on the user rights
		if(!file->doAccessCheck(access)) //If the access check fails send the appropriate response and quit the handler
			return SendAccessDenied(downstream_);
		
		nlohmann::json js;
		if(_user)
		{
			nlohmann::json usr;
			usr["email"] = _user->GetEmail();
			usr["access"] = _user->GetAccessRights();
			js["user"] = std::move(usr);
		}
		if(file->getPath() == "web/Search.html")
		{
			js["pillars"] = Zotero::GetPillars();
		}


		//Build the response from the found file
		ResponseBuilder resp(downstream_);
			resp.status(200, "OK")
			.header("Content-Type",file->getMimeType())
			.body(file->getBuffer());
		if(file->getMimeType()=="text/html")
			resp.body(URIFile::PrepareDataForSending(std::move(js)));

		resp.sendWithEOM();
	}
	catch(...)
	{
		alx::cout.write(alx::console::red_black,"Could not satisfy request to: ",headers->getURL(),"\n");
		//Some error occured just state that the file was not found
		return SendErrorNotFound(downstream_);
	}
}

