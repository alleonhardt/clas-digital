#include "src/login/user_system.hpp"
#include <fstream>

#ifdef _DEBUG_
#define DBG_MSG debug::print
#else
#define DBG_MSG debug::empty
#endif

UserHandler::UserHandler(std::string filePath)
{
	DBG_MSG("Trying to load user table: ",filePath);
}

bool UserHandler::AddUser(std::string email, std::string password, int access)
{
	std::unique_lock lck(m);
	DBG_MSG("Searching for user: ",email);
	if(m_userTable.find(email)==m_userTable.end())
	{
		DBG_MSG("Could not find user!");
		return false;
	}
	DBG_MSG("Created new user: ",email," with access: ",access," and password: ",password);
	m_userTable[email] = std::move(std::make_shared<User>(email.c_str(),password.c_str(),access));
	return true;
}

void UserHandler::SetAccessRights(std::string email, int newAccess)
{
	std::shared_lock lck(m);
	DBG_MSG("Set access rights of user: ",email," to: ",newAccess);
	auto it = m_userTable.find(email);
	if(it==m_userTable.end())
		return;
	it->second->SetAccessRights(newAccess);
}

std::string UserHandler::toJSON()
{
	std::unique_lock lck(m);
	return "";
}

void UserHandler::RemoveUser(std::string email)
{
	std::unique_lock lck(m);
	m_userTable.erase(email);
}

std::string UserHandler::DoLogin(std::string email, std::string password)
{
	std::string sessid=email;
	
	std::shared_lock lck(m);
	auto it = m_userTable.find(email);
	if(it==m_userTable.end())
		return "";

	if(it->second->GetSessid()!="")
		return "";
	
	std::ifstream ifs("/dev/urandom",std::ios::in|std::ios::binary);
	if(!ifs)
		throw std::runtime_error("Could not generate session id!");
	char buffer[33];
	//read 32 random bytes
	ifs.read(buffer,32);
	ifs.close();
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"_";
	sessid+="::";
	//Assign each character in the buffer a random char based on the byte
	//read from the random device
	for(int i = 0; i< 32; ++i)
		sessid += alphanum[buffer[i] % (sizeof(alphanum)-1)];
	
	it->second->SetSessionId(sessid);
	return std::move(sessid);
}

std::shared_ptr<User> UserHandler::GetUserBySessid(std::string x)
{
	auto pos = x.find("::");
	if(pos==std::string::npos)
		return std::shared_ptr<User>(nullptr);
	auto email = x.substr(0,pos);

	std::shared_lock lck(m);
	auto it = m_userTable.find(email);
	if(it==m_userTable.end() || it->second->GetSessid()!=x)
		return std::shared_ptr<User>(nullptr);
	return it->second;

}

void UserHandler::RemoveSession(std::string x)
{
	std::shared_ptr<User> ptr = std::move(GetUserBySessid(x));
	ptr->SetSessionId("");
}


#ifdef COMPILE_UNITTEST

TEST(UserTable, load)
{
	UserHandler tabl("bin/usertable.json");
}

#endif
