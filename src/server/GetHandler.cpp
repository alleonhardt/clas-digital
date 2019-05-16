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
	{"/home",URIFile("web/index.html",1)}
};



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

#ifdef COMPILE_UNITTEST

TEST(URIFile, load_file)
{
	URIFile fl("web/index.html",0);
	std::ifstream file("web/index.html", std::ios::binary);
	if(!file)
	{
		ASSERT_EQ(true,false);
	}

	file.seekg(0,file.end);
	auto fileSize = file.tellg();
	char *buff = new char[fileSize];
	//Clear the eof bit of filestream
	file.clear();
	//go to the beginning of the file again
	file.seekg(0, file.beg);
	//Read the file into the buffer
	file.read(buff,fileSize);

	//Expect the content to be the same
	EXPECT_EQ(memcmp(fl.getBufferReference()->data(),buff,fileSize),0);
	//File size and length of the buffer should be the same
	EXPECT_EQ(fl.getBufferReference()->length(),fileSize);

	std::unique_ptr<folly::IOBuf> file2 = std::move(fl.getBufferReference());
	//Expect the buffer to be moved inside of the new file
	EXPECT_EQ(memcmp(file2->data(),buff,fileSize),0);
	//File size and length of the buffer should be the same
	EXPECT_EQ(file2->length(),fileSize);



	//Expect on getBufferReference to reload the file to provide the data
	EXPECT_EQ(memcmp(fl.getBufferReference()->data(),buff,fileSize),0);
	//File size and length of the buffer should be the same
	EXPECT_EQ(fl.getBufferReference()->length(),fileSize);
	delete []buff;
}

TEST(URIFile, load_bad_file)
{
	try
	{
		//bin is a directory not a file!
		URIFile fl("bin");
		ASSERT_EQ(true, false);
	}
	catch(...)
	{
	
		//Function throws 0 if the file is not good check if this is our exception or something else{
		EXPECT_EQ(0,0);
	}

	try
	{
		//bin does exist as folder but the file does not exist within
		URIFile fl("bin/whatever.bin");
		ASSERT_EQ(true,false);
	}
	catch(...)
	{
	
		//Function throws 0 if the file is not good check if this is our exception or something else{
		EXPECT_EQ(0,0);
	}

	try
	{
		//The file and the folder does not exist
		URIFile fl("bin/what/store/bin.bin");
		ASSERT_EQ(true,false);
	}
	catch(...)
	{
		//Function throws 0 if the file is not good check if this is our exception or something else
		EXPECT_EQ(0,0);
	}
}

TEST(URIFile,access_check)
{
	URIFile fl("web/index.html",1);
	EXPECT_EQ(fl.doAccessCheck(0),false);
	EXPECT_EQ(fl.doAccessCheck(1),true);
	EXPECT_EQ(fl.doAccessCheck(255),true);
}

#endif





GetHandler::GetHandler(User *from) : EmptyHandler(from)
{}

void GetHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
	std::cout<<"Servicing request to url : "<< headers->getURL() <<std::endl;
	//As long as we dont have users yet just set the access here
	int access = 0xFF;
	try
	{
		//Have a look if the file requested does exist at all
		URIFile &file = fileAccess.at(headers->getPath());

		//Ok the file does exist so do the access check based on the user rights
		if(!file.doAccessCheck(access))
		{
			//If the access check fails send the appropriate response and quit the handler
			SendAccessDenied(downstream_);
			return;
		}
		
		//Build the response from the found file
		ResponseBuilder(downstream_)
			.status(200, "OK")
			.header("Content-Type",file.getMimeType())
			.body(file.getBuffer())
			.sendWithEOM();

	}
	catch(...)
	{
		//Some error occured just state that the file was not found
		SendErrorNotFound(downstream_);
		return;
	}
}

