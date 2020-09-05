#include "user.hpp"
#include <debug/debug.hpp>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <random>



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
    str += alphanum[dev()%sizeof(alphanum)];
  return str;
}


User::User(std::string email, UserAccess acc) : email_(email),access_(acc) {}

std::string User::Email() {
  return email_;
}

UserAccess User::Access() {
  return access_;
}




UserTable::UserTable() : connection_(nullptr) {}

UserTable::ReturnCodes UserTable::Load(std::filesystem::path database_path) {
  try {
    std::lock_guard lck(class_lock_);
    connection_ = new SQLite::Database(database_path.string().c_str(),SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
    SQLite::Statement query(*connection_,"SELECT name FROM sqlite_master WHERE type='table' AND name='users';");
    bool exists = false;
    while(query.executeStep()) {
      exists = true;
    }

    if(!exists) {
      SQLite::Transaction act(*connection_);
      connection_->exec( "CREATE TABLE users("
                  "email TEXT PRIMARY KEY NOT NULL,"
                  "password TEXT NOT NULL,"
                  "name TEXT NOT NULL,"
                  "access INT);");

      std::string cmd = "INSERT INTO USERS(email,password,name,access) VALUES ('root','";
      cmd+=sha3_512("password");
      cmd+="','Admin',7);";

      connection_->exec(cmd.c_str());

      act.commit();
      debug::log(debug::LOG_DEBUG, "Created the table users and the root user.\n");
    }
    
  }
  catch(SQLite::Exception &e) {
    debug::log(debug::LOG_ERROR,"Caught exception in ",__FUNCTION__," error \"",e.getErrorStr(),"\"\n");
    return ReturnCodes::UNKNOWN_ERROR;
  }
  return ReturnCodes::OK;
}

UserTable::ReturnCodes UserTable::AddUser(std::string email, std::string password, std::string name, UserAccess acc) {
  try {
    std::lock_guard lck(class_lock_);
    SQLite::Statement insert_cmd(*connection_, "INSERT INTO users(email,password,name,access) "
                                  "VALUES (?,?,?,?);");
    insert_cmd.bind(1,email.c_str());
    insert_cmd.bind(2,sha3_512(password).c_str());
    insert_cmd.bind(3,name.c_str());
    insert_cmd.bind(4,(int)acc);
    insert_cmd.exec();
    debug::log(debug::LOG_DEBUG,"Added new user \"",email,"\" with access \"",(int)acc,"\"\n");
  }
  catch(SQLite::Exception &e) {
    debug::log(debug::LOG_ERROR,"Could not add user \"",email,"\" to table users. Error string: \"",e.getErrorStr(),"\"\n");
    
    if(e.getExtendedErrorCode() == SQLITE_CONSTRAINT_PRIMARYKEY)
      return ReturnCodes::USER_EXISTS;

    return ReturnCodes::UNKNOWN_ERROR;
  }
  return ReturnCodes::OK;
}

UserTable::ReturnCodes UserTable::RemoveUser(std::string email)
{
  try {
    std::lock_guard lck(class_lock_);
    SQLite::Statement insert_cmd(*connection_, "DELETE FROM users WHERE email=?;");

    insert_cmd.bind(1,email.c_str());
    insert_cmd.exec();
    debug::log(debug::LOG_DEBUG,"Removed new user \"",email,"\"\n");
  }
  catch(SQLite::Exception &e) {

    debug::log(debug::LOG_ERROR,"Could not remove user \"",email,"\" from table users. Error string: \"",e.getErrorStr(),"\"\n");
    return ReturnCodes::UNKNOWN_ERROR;
  }
  return ReturnCodes::OK;
}


int UserTable::GetNumUsers() {
  try {
    std::lock_guard lck(class_lock_);
    SQLite::Statement st(*connection_,"SELECT COUNT(*) FROM users;");
    
    st.executeStep();
    return st.getColumn(0).getInt();
  }
  catch(SQLite::Exception &e) {
    debug::log(debug::LOG_ERROR,"Could not get number of users! Error string: \"",e.getErrorStr(),"\"\n");
  }

  return -1;
}

std::string UserTable::LogIn(std::string email, std::string password)
{
  try {
    std::lock_guard lck(class_lock_);
    SQLite::Statement insert_cmd(*connection_, "SELECT email,access FROM users WHERE email=? AND password=?");
    insert_cmd.bind(1,email.c_str());
    insert_cmd.bind(2,sha3_512(password).c_str());
    insert_cmd.executeStep();
    User us(insert_cmd.getColumn(0).getString(),(UserAccess)insert_cmd.getColumn(1).getInt());
    std::string cookie = CreateRandomString(32);
    logged_in_.insert({cookie, us});
    return cookie;
  }
  catch(SQLite::Exception &e) { }
  return "";
}

User *UserTable::GetUserFromCookie(const std::string &cookie) {
  try {
    std::lock_guard lck(class_lock_);
    return &logged_in_.at(cookie);
  }
  catch(...) { }
  return nullptr;
}


void UserTable::RemoveCookie(const std::string &cookie) {
 try {
    std::lock_guard lck(class_lock_);
    if(logged_in_.size() == 1)
      logged_in_.clear();
    else
      logged_in_.erase(cookie);
  }
  catch(...) { }
}


UserTable::~UserTable() {
  std::lock_guard lck(class_lock_);
  if(connection_)
    delete connection_;
}
