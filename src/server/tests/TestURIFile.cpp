#include "src/server/BasicHandlers.hpp"
#include <gtest/gtest.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/FileUtil.h>
#include <folly/executors/GlobalExecutor.h>
#include <ctime>
#include <fstream>
#include <string_view>
#include <unordered_map>



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
	//Tries to open a folder
	EXPECT_ANY_THROW(URIFile fl("bin"));

	//bin does exist as folder but the file does not exist within
	EXPECT_ANY_THROW(URIFile fl("bin/whatever.bin"));

	//The file and the folder does not exist
	EXPECT_ANY_THROW(URIFile fl("bin/what/store/bin.bin"));
}

TEST(URIFile,access_check)
{
	URIFile fl("web/index.html",1);
	EXPECT_EQ(fl.doAccessCheck(0),false);
	EXPECT_EQ(fl.doAccessCheck(1),true);
	EXPECT_EQ(fl.doAccessCheck(255),true);
}

