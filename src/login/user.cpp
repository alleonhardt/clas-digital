#include "src/login/user_system.hpp"

User::User() : _password(""),_email("guest"),_accessRights(0)
{}

User::User(const char *email, const char *pass, int access) : _password(pass),_email(email),_accessRights(access) 
{}

std::string User::toJSON() const
{
	//Tries to create {"email": "email address","access": access rights}
	std::string ret = "{\"email\": \"";
	ret+=_email;
	ret+="\",\"access\": ";
	ret+=std::to_string(_accessRights);
	ret+="}";
	return std::move(ret);
}

int User::GetAccessRights() const
{
	return reinterpret_cast<int>(_accessRights);
}

void User::SetAccessRights(int acc)
{
	_accessRights = acc;
}


const std::string &User::GetEmail() const
{
	return _email;
}

const std::string &User::GetPassword() const
{
	return _password;
}

bool User::DoesMatch(std::string email, std::string passwd) const
{
	if(_email==email && _password == passwd)
		return true;
	else
		return false;
}

const std::string &User::GetSessid() const
{
	return _sessid;
}


void User::SetSessionId(std::string sessid)
{
	_sessid = std::move(sessid);
}


#ifdef COMPILE_UNITTEST

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


#endif
