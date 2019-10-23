#include "login/user_system.hpp"
#include <fstream>

UserHandler::UserHandler(std::string filePath)
{
	nlohmann::json js; //The json file interface
	//Load the file from the specified path formatted in the json format
	std::ifstream isf(filePath.c_str(), std::ios::in);
	//A user table is crucial for starting the server so quit the server if there isnt one
	if(!isf)
	{
		nlohmann::json entry;
		entry["password"] = "password";
		entry["access"] = 7;
		entry["email"] = "root@root.de";
		js.push_back(std::move(entry));
		std::cout<<"Working with default user table, this is dangerous! Standard user: root@root.de, password: password"<<std::endl;
	}
	else
	{
		isf>>js;
	}
	isf.close();

	//Load all entrys from the json file into memory
	for(auto &entry : js)
		if(!AddUser(entry["email"].get<std::string>(),entry["password"].get<std::string>(),entry["access"].get<int>()))
		{
			throw 0;
		}
}


UserHandler::~UserHandler()
{
	std::ofstream os(USRTABLE_PATH,std::ios::out);
	if(os)
	{
		nlohmann::json js;
		for(auto &it : m_userTable)
		{
			nlohmann::json entry;
			entry["email"] = it.second->GetEmail();
			entry["password"] = it.second->GetPassword();
			entry["access"] = it.second->GetAccessRights();
			js.push_back(entry);
		}
		std::cout<<"Writing the current user table to disk!"<<std::endl;
		os<<js.dump();
	}
	os.close();
}


/*
 *
 * #include <openssl/sha.h>

std::string simpleSHA512(const char* input, unsigned long length)
{
	std::string ret;
	
	unsigned char md[SHA512_DIGEST_LENGTH+1];
	md[SHA512_DIGEST_LENGTH] = '0';

    SHA512_CTX context;
    if(!SHA512_Init(&context))
        return std::move(ret);

    if(!SHA512_Update(&context, (unsigned char*)input, length))
        return std::move(ret);

    if(!SHA512_Final(md, &context))
        return std::move(ret);

    return std::move(ret);
}
*/

bool UserHandler::AddUser(std::string email, std::string password, int access)
{
	//Create a unique lock because the map is going to be modified critical section from here on
	std::unique_lock lck(m);
	//Check if the user already exists
	if(m_userTable.find(email)!=m_userTable.end())
		return false;	//User exists tell the user about the aborted operation
	//Create the user with the specific email password and access
	m_userTable[email] = std::move(std::make_shared<User>(email.c_str(),password.c_str(),access));
	//Tell the user about the successfull operation
	return true;
}

void UserHandler::SetAccessRights(std::string email, int newAccess)
{
	//Shared lock as nothing crucial is going to be modified
	std::shared_lock lck(m);
	//Search for the user
	auto it = m_userTable.find(email);
	if(it==m_userTable.end())
		return; //No user just return and dont change anything
	//We got a user so change the access rights
	it->second->SetAccessRights(newAccess);
}

std::string UserHandler::toJSON()
{
	std::string json="[";
	std::shared_lock lck(m);
	for(auto &it : m_userTable)
	{
		//Put the email and the access rights into the specific json
		json+="{\"email\":\"";
		json+=it.second->GetEmail();
		json+="\",\"access\":";
		json+=std::to_string(it.second->GetAccessRights());
		json+="},";
		//The list probably goes on so put a , at the end for the next entry
	}
	//Replace the , by the ] to close the list
	json[json.length()-1] = ']';

	return std::move(json);
}

void UserHandler::RemoveUser(std::string email)
{
	//Change the table so lock the map entirely to prevent segmentation faults
	std::unique_lock lck(m);

	auto it = m_userTable.find(email);
	if(it==m_userTable.end())
		return;
	it->second->SetAccessRights(0);
	//erase the user doesnt do anything if the user does not exist
	m_userTable.erase(it);
}

std::string UserHandler::DoLogin(std::string email, std::string password)
{
	//First create a new session id for the user all session ids are in the format
	//user_email::access:::"Random String"
	std::string sessid=email;
	
	//Collect 32 random bytes in Linux provided by /dev/urandom
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
	

	//Now acquire the unique lock to change the user table
	std::unique_lock lck(m);
	//Search for the user if there is no one give back an empty session id
	auto it = m_userTable.find(email);
	if(it==m_userTable.end())
		return "";
	if(!it->second->DoesMatch(email,password))
		return "";
	sessid+=std::to_string(it->second->GetAccessRights());
	sessid+=":::";

	//Assign each character in the buffer a random char based on the byte
	//read from the random device
	for(int i = 0; i< 32; ++i)
		sessid += alphanum[buffer[i] % (sizeof(alphanum)-1)];

	//Set the session id to the new value, will kick any user that is singed in twice
	it->second->SetSessionId(sessid);
	//return the generated session id
	return std::move(sessid);
}

std::shared_ptr<User> UserHandler::GetUserBySessid(std::string x)
{
	//x is in the session id format meaning: user_email::access:::"Random String"
	auto pos = x.find("::");
	if(pos==std::string::npos)
		return std::shared_ptr<User>(nullptr);

	//Extract the user email
	auto email = x.substr(0,pos);

	//Accquire a shared lock and find the user in the table
	std::shared_lock lck(m);
	auto it = m_userTable.find(email);
	//If the user session id is not the same as in the user or if the user
	//does not exist return an empty user
	if(it==m_userTable.end() || it->second->GetSessid()!=x)
		return std::shared_ptr<User>(nullptr);
	//return the user fitting to the session id
	return it->second;

}

void UserHandler::RemoveSession(std::string x)
{
	//No lock before this functioN!!!!!! This will result in a deadlock
	std::shared_ptr<User> ptr = std::move(GetUserBySessid(x));
	if(ptr)
	{
		//Lock the function now as we are changing something important
		std::unique_lock lck(m);
		
		//Reset the session id so the user cant use the session id anymore
		ptr->SetSessionId("");
	}
}

std::shared_ptr<User> UserHandler::GetUserByName(std::string email)
{
	//Accquire a shared lock as only reading is done and no writing
	std::shared_lock lck(m);
	try
	{
		//return the user found if there is any
		return m_userTable.at(email);
	}
	catch(...)
	{
		//If there is no user with this email return an nullptr
		return std::shared_ptr<User>(nullptr);
	}
}
