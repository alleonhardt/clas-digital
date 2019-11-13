#include "login/user_system.hpp"

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
	return ret;
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

bool User::AccessCheck(const std::shared_ptr<User> &usr, int accRequired)
{
    std::cout<<"Access check from usr "<<usr.get()<<std::endl;
    if(usr)
    {
	std::cout<<"Access: "<<usr->GetAccessRights()<<std::endl;
	std::cout<<"Access required = "<<accRequired<<std::endl;
    }
	if(accRequired==0)
		return true;
	else if(!usr||((usr->GetAccessRights()&accRequired)!=accRequired))
		return false;

	std::cout<<"Access granted"<<std::endl;
	return true;
}

