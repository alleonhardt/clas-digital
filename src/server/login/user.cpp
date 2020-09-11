#include "user.hpp"
#include <debug/debug.hpp>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <random>
#include "filehandler/util.h"

using namespace clas_digital;

namespace clas_digital
{
  std::string sha3_512(const std::string& input)
  {
    unsigned int digest_length = SHA512_DIGEST_LENGTH;
    const EVP_MD* algorithm = EVP_sha3_512();
    uint8_t* digest = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
    debug::CleanupDtor dtor([digest](){OPENSSL_free(digest);});


    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, algorithm, nullptr);
    EVP_DigestUpdate(context, input.c_str(), input.size());
    EVP_DigestFinal_ex(context, digest, &digest_length);
    EVP_MD_CTX_destroy(context);

    std::stringstream stream;

    for(auto b : std::vector<uint8_t>(digest,digest+digest_length))
      stream << std::setw(2) << std::hex << std::setfill('0') << b;

    return stream.str();
  }

  std::string CreateRandomString(int len) {
    std::string str;
    static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "_";
    std::random_device dev;
    //Create random alphanumeric cookie
    for(int i = 0; i < len; i++)
      str += alphanum[dev()%(sizeof(alphanum)-1)];
    return str;
  }
}

bool User::ConstructFromJSON(const nlohmann::json &js, ConstructFlags flags)
{
  email_ = js.value("email","");
  password_ = js.value("password","");

  if(flags == CONSTRUCT_NEW_USER)
    password_ = sha3_512(password_);

  access_ = js.value("access",0);
  return email_ != "";
}

std::string User::GetPrimaryKey()
{
  return email_;
}

nlohmann::json User::ExportToJSON(ExportPurpose purpose)
{

  nlohmann::json js;
  js["email"] = email_;
  
  if(purpose == User::EXPORT_FOR_SAVING_TO_FILE)
    js["password"] = password_;

  js["access"] = access_;
  return js;
}

void User::UpdateFromJSON(const nlohmann::json &js)
{
  std::string passw = js.value("password","");
  if(passw=="")
    password_ = sha3_512(passw);

  int acc = js.value("access",-1);
  if(acc!=-1)
    access_ = acc;
}

bool User::CheckCredentials(nlohmann::json options)
{
  if(options.value("email","") == email_  && sha3_512(options.value("password","")) == password_)
    return true;
  else return false;
}

bool User::DoAccessCheck(std::string URI)
{
  //Always allow access
  return true;
}

User::User()
{}

User::User(std::string email) : email_(email)
{
}






UserTable::UserTable()
{
  create_user_ = [](){
    return new User;
  };

  primary_key_field = "email";
}

UserTable::~UserTable()
{
  SaveUserTable();
}

void UserTable::SetCreateUserCallback(std::function<IUser*()> fnc)
{
  create_user_ = fnc;
}

void UserTable::SetPrimaryKeyFieldName(std::string primary_key_field_name)
{
  primary_key_field = primary_key_field_name;
}


debug::Error<UserTable::ReturnValues> UserTable::Load()
{
  save_file_ = "";
  return debug::Error(RET_OK);
}

debug::Error<UserTable::ReturnValues> UserTable::Load(std::filesystem::path database_path)
{
  if(database_path.string() == ":memory:")
    return Load();

  std::ifstream ifs(database_path.c_str(), std::ios::in);
  if(!ifs.is_open())
  {
    save_file_ = database_path;
    return debug::Error(RET_CANT_OPEN_USER_FILE,"Cant open user file at "+database_path.string());
  }

  nlohmann::json js;
  try
  {
    ifs>>js;
  }
  catch(...)
  {
    return debug::Error(RET_USER_FILE_CORRUPTED,"The user file does not contain valid json statements, the file is corrupted. File at " +database_path.string());
  }
  ifs.close();

  for(auto &it : js)
  {
    auto err = __AddUser(it,IUser::ConstructFlags::CONSTRUCT_LOAD_FROM_FILE);
    if(err) return err;
  }
  save_file_ = database_path;
  return RET_OK;
}

debug::Error<UserTable::ReturnValues> UserTable::Update(nlohmann::json update_info)
{
  std::string email = update_info.value(primary_key_field,"");
  if(email=="")
    return debug::Error(RET_MISSING_PRIMARY_KEY,"Missing primary key field in json. Primary key field name: "+primary_key_field);

  auto ptr = GetUserFromPrimaryKey(email);
  if(ptr)
  {
    ptr->UpdateFromJSON(update_info);
    return debug::Error(RET_OK);
  }

  return debug::Error(RET_USER_DOES_NOT_EXIST,"Cant change user attributes as the user does not exist. User: "+email);
}

debug::Error<UserTable::ReturnValues> UserTable::AddUser(nlohmann::json create_credentials)
{
  return __AddUser(std::move(create_credentials), IUser::ConstructFlags::CONSTRUCT_NEW_USER);
}

debug::Error<UserTable::ReturnValues> UserTable::__AddUser(nlohmann::json create_credentials, User::ConstructFlags flags)
{
  std::unique_lock lock(users_lock_);
  auto usr = create_user_();
  auto ret = usr->ConstructFromJSON(create_credentials,flags);
  if(!ret)
    return debug::Error(RET_USER_FILE_CORRUPTED,"The user credentials are corrupted cant call ConstructFromJSON() on user\n");

  if(users_.count(usr->GetPrimaryKey())>0)
    return debug::Error(RET_USER_EXISTS,"The user exists already primary constraint failed\n");

  users_.insert({usr->GetPrimaryKey(),std::shared_ptr<IUser>(usr)});
  return debug::Error(RET_OK);
}



debug::Error<UserTable::ReturnValues> UserTable::RemoveUser(nlohmann::json remove_info)
{
  std::string primary_key = remove_info.value(primary_key_field,"");
  return RemoveUserByKey(primary_key);
}

debug::Error<UserTable::ReturnValues> UserTable::RemoveUserByKey(std::string primary_key)
{
  std::unique_lock lock(users_lock_);
  if(users_.count(primary_key)>0)
  {
    if(users_.size() == 1)
      users_.clear();
    else
      users_.erase(primary_key);
    return debug::Error(RET_OK);
  }
  return debug::Error(RET_USER_DOES_NOT_EXIST,"Could not find and delete the user "+primary_key);
}

std::string UserTable::LogIn(nlohmann::json credentials)
{
  std::shared_ptr<IUser> shared_user = GetUserFromPrimaryKey(credentials.value(primary_key_field,""));
  
  if(shared_user && shared_user->CheckCredentials(credentials))
  {
    std::unique_lock lock(loggedin_lock_);

    while(true)
    {
      std::string cookie = CreateRandomString(32);
      if(logged_in_.count(cookie) == 0)
      {
        logged_in_.insert({cookie,shared_user});
        return cookie;
      }
    }
  }
  return "";
}

std::shared_ptr<IUser> UserTable::GetUserFromPrimaryKey(std::string key)
{
  try
  {
    std::shared_lock lock(loggedin_lock_);
    return users_.at(key);
  }
  catch(...)
  {}
  return nullptr;
}

std::shared_ptr<IUser> UserTable::GetUserFromCookie(const std::string &cookie)
{
  std::shared_lock lock(loggedin_lock_);
  try
  {
    return logged_in_.at(cookie);
  }
  catch(...){}

  return nullptr;
}

void UserTable::RemoveCookie(const std::string &cookie)
{
  std::unique_lock lock(users_lock_);
  if(logged_in_.count(cookie)>0)
  {
    if(logged_in_.size() == 1)
      logged_in_.clear();
    else
      logged_in_.erase(cookie);
  }
}


int UserTable::GetNumUsers() const
{
  return users_.size();
}

nlohmann::json UserTable::GetAsJSON()
{
  std::unique_lock lock(users_lock_);
  nlohmann::json js;
  for(auto &it : users_)
  {
    js.push_back(it.second->ExportToJSON(IUser::EXPORT_FOR_SENDING_TO_ADMIN));
  }
  return js;
}

debug::Error<UserTable::ReturnValues> UserTable::SaveUserTable()
{
  if(save_file_.string() != "")
  {
    if(!clas_digital::atomic_write_file(save_file_,GetAsJSON()))
      return debug::Error(RET_CANT_OPEN_USER_FILE,"Cannot open user file for saving. Filename: "+save_file_.string());
  }
  return debug::Error(RET_OK);
}

