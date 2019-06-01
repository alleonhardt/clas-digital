#include "src/zotero/zotero.hpp"
#include <gtest/gtest.h>

TEST(Zotero,init)
{
	EXPECT_EQ(true,true);
}

/**
 * This test should check if the zotero connection works by doing a simple
 * request and searching for some key which is always present in this request.
 */
/*
TEST(Zotero,zotero_test)
{
	//Create the connection to the server
	Zotero zot;

	//Do the most basic request in zotero
	std::string request = std::move(zot.SendRequest(Zotero::Request::GetAllPillars));
	//Parse the string to a json and search for the unique key always
	//present in this request
	auto js = nlohmann::json::parse(request);
	bool exist = false;
	for(auto &el : js.items())
	{
		if(el.value()["key"].get<std::string>()=="2SWXSFCX")
			exist = true;
	}
	//The key should be present
	ASSERT_EQ(exist,true);
}*/

