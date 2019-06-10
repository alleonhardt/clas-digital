#include "src/zotero/zotero.hpp"
#include <gtest/gtest.h>
#include <iostream>

/**
 * This test should check if the zotero connection works by doing a simple
 * request and searching for some key which is always present in this request.
 */

TEST(Zotero,zotero_test)
{
	std::string request;
	request = std::move(Zotero::SendRequest(Zotero::Request::GetAllPillars));

	if(request=="")
	{
		char c;
		std::cout<<"Are you connected to the internet at the moment?(y/n): ";
		std::cin>>c;
		ASSERT_NE(c,'y');
		return;
	}

	//Parse the string to a json and search for the unique key always
	//present in this request
	nlohmann::json js;
	ASSERT_NO_THROW(js = nlohmann::json::parse(request));
	bool exist = false;
	for(auto &el : js.items())
	{
		if(el.value()["key"].get<std::string>()=="2SWXSFCX")
			exist = true;
	}
	//The key should be present
	ASSERT_EQ(exist,true);
}

