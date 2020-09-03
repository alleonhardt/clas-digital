#include "user.hpp"
#include <debug/debug.hpp>

UserTable::UserTable() {

  user_database_ = nullptr;

}

bool UserTable::Load(std::filesystem::path database_path) {

  bool new_database_exists = std::filesystem::exists(database_path);
  int err = sqlite3_open(database_path.string().c_str(),&user_database_);
  char *gErrorMessage = nullptr;

  if (err) {
    debug::log(debug::LOG_LEVEL::ERRORS,"Could not open user table database! Error string %s\n",sqlite3_errmsg(user_database_));
    return false;
  }

  if(!new_database_exists) {
    debug::log(debug::LOG_LEVEL::WARNING,"Could not open user table database at %s, creating new one!\n",database_path.string().c_str());


    const char create_user_table_command[] = "CREATE TABLE USERS("
                                              "EMAIL CHAR(60) PRIMARY KEY NOT NULL,"
                                              "PASSWORD CHAR(64) NOT NULL,"
                                              "NAME CHAR(60) NOT NULL,"
                                              "ACCESS INT);";

    err = sqlite3_exec(user_database_, create_user_table_command, nullptr, 0, &gErrorMessage);
   
   if ( err != SQLITE_OK ) {
     debug::log(debug::LOG_LEVEL::ERRORS,"Could not create table users! Error string %s\n",gErrorMessage);
      sqlite3_free(gErrorMessage);
      return false;
   }
    
   debug::log(debug::LOG_LEVEL::DEBUG,"Successfully created the table users in the database");


   const char create_root_command[] = "INSERT INTO USERS(EMAIL,PASSWORD,NAME,ACCESS) "
                                      "VALUES ('root','password','Admin',7);";
  
   err = sqlite3_exec(user_database_, create_root_command, nullptr, 0, &gErrorMessage);
   
   if ( err != SQLITE_OK ) {
     debug::log(debug::LOG_LEVEL::ERRORS,"Could not create root user! Error string %s\n",gErrorMessage);
      sqlite3_free(gErrorMessage);
      return false;
   }
  
   debug::log(debug::LOG_LEVEL::DEBUG,"Successfully created the root user.\n");
  }
   return true;
}

UserTable::~UserTable() {
  if(user_database_)
  {
    sqlite3_close(user_database_);
    user_database_ = nullptr;
  }
}
