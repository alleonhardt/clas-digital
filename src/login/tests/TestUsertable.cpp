#include "src/login/user_system.hpp"
#include <gtest/gtest.h>

TEST(UserTable, load)
{
	std::unique_ptr<UserHandler> m_ptr;
	try
	{
		m_ptr = std::make_unique<UserHandler>("bin/usertable.json");
	}
	catch(...)
	{
		ASSERT_EQ(true,false);
	}

	if(m_ptr->GetUserByName("equinox.malexander@gmail.com") != std::shared_ptr<User>(nullptr))
	{
		ASSERT_EQ(true,false);
	}

	m_ptr->AddUser("equinox.malexander@gmail.com","pass",7);
	if(m_ptr->GetUserByName("equinox.malexander@gmail.com") == std::shared_ptr<User>(nullptr))
	{
		ASSERT_EQ(false,true);
	}
	auto user = m_ptr->GetUserByName("equinox.malexander@gmail.com");
	EXPECT_EQ(user->GetAccessRights(),7);
	m_ptr->SetAccessRights(user->GetEmail(),5);
	EXPECT_EQ(user->GetAccessRights(),5);
}
