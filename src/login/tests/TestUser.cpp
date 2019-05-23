#include "src/login/user_system.hpp"
#include <gtest/gtest.h>

TEST(UserClass,BasicFunctions)
{
	User us("hello","world",7);

	EXPECT_EQ(us.GetAccessRights(),7);
	us.SetAccessRights(5);
	EXPECT_EQ(us.GetAccessRights(),5);
}

TEST(UserClass,MatchFunction)
{
	User us("hello","world",7);
	EXPECT_EQ(us.DoesMatch("hello","world"),true);
	EXPECT_EQ(us.DoesMatch("hello1","world"),false);
	EXPECT_EQ(us.DoesMatch("hello","world1"),false);
	EXPECT_EQ(us.DoesMatch("whatever","whatever2"),false);
}

TEST(UserClass,to_json)
{
	User us("email","password",10);
	std::string str = us.toJSON();
	EXPECT_EQ(str,"{\"email\": \"email\",\"access\": 10}");
	us.SetAccessRights(9);
	EXPECT_EQ(us.toJSON(),"{\"email\": \"email\",\"access\": 9}");
}

