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

TEST(UserTable,delete_add)
{
	std::unique_ptr<UserHandler> m_ptr;
	ASSERT_NO_THROW(m_ptr = std::make_unique<UserHandler>("bin/usertable.json"));
	
	m_ptr->RemoveUser("equinox.salexander@gmail.com");
	ASSERT_EQ(m_ptr->GetUserByName("equinox.salexander@gmail.com"),nullptr);

	m_ptr->AddUser("equinox.salexander@gmail.com","password",7);
	ASSERT_NE(m_ptr->GetUserByName("equinox.salexander@gmail.com"),nullptr);
	ASSERT_EQ(m_ptr->AddUser("equinox.salexander@gmail.com","whatever",9),false);
}

TEST(UserTable,sessidtest)
{
	std::unique_ptr<UserHandler> m_ptr;
	ASSERT_NO_THROW(m_ptr = std::make_unique<UserHandler>("bin/usertable.json"));
	
	ASSERT_EQ(m_ptr->DoLogin("what@gmail.com","testpass"),"");
	ASSERT_EQ(m_ptr->DoLogin("equinox.salexander@gmail.com","passwor"),"");
	auto res = m_ptr->DoLogin("equinox.salexander@gmail.com","password");
	ASSERT_NE(res,"");
	auto usr = m_ptr->GetUserBySessid(res);
	ASSERT_NE(usr,nullptr);
	auto usr2 = m_ptr->GetUserByName("equinox.salexander@gmail.com");
	EXPECT_EQ(usr->GetAccessRights(),usr2->GetAccessRights());
	EXPECT_EQ(usr->GetSessid(), usr2->GetSessid());
	EXPECT_EQ(usr->GetEmail(), usr2->GetEmail());
	EXPECT_EQ(usr->GetPassword(),usr2->GetPassword());
}
