#include "user.hpp"
#include <debug/debug.hpp>

UserTable::UserTable() : initialised_(false) {}


UserTable::ReturnCodes UserTable::Load(std::filesystem::path database_path) {
  static std::mutex m;
  std::lock_guard lck(m);
  try {
    SQLite::Database *data = new SQLite::Database(database_path.string().c_str(),SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
    SQLite::Statement query(*data,"SELECT name FROM sqlite_master WHERE type='table' AND name='users';");
    bool exists = false;
    while(query.executeStep()) {
      exists = true;
    }

    if(!exists) {
      SQLite::Transaction act(*data);
      data->exec( "CREATE TABLE users("
                  "email TEXT PRIMARY KEY NOT NULL,"
                  "password TEXT NOT NULL,"
                  "name TEXT NOT NULL,"
                  "access INT);");
      data->exec( "INSERT INTO USERS(email,password,name,access) "
                  "VALUES ('root','password','Admin',7);");
      data->exec( "CREATE TABLE logged_in("
                  "cookie_hash TEXT PRIMARY KEY NOT NULL,"
                  "email TEXT,"
                  "FOREIGN KEY(email) REFERENCES users(email));");

      act.commit();
      debug::log(debug::LOG_DEBUG, "Created the table users, logged_in and created the root user!\n");
    }
    connections_.push(data);

    for(unsigned int i = 0; i < std::thread::hardware_concurrency()-1; i++) {
      connections_.push(new SQLite::Database(database_path.string().c_str(),SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE));
    initialised_ = true;
    
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
    auto connection = connections_.pop();
    debug::CleanupDtor dt([connection,this](){this->connections_.push(connection);});


    SQLite::Statement insert_cmd(*connection, "INSERT INTO users(email,password,name,access) "
                                  "VALUES (?,?,?,?);");
    insert_cmd.bind(1,email.c_str());
    insert_cmd.bind(2,password.c_str());
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
    auto connection = connections_.pop();
    debug::CleanupDtor dt([connection,this](){this->connections_.push(connection);});
    SQLite::Statement insert_cmd(*connection, "DELETE FROM users WHERE email=?;");

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


UserTable::~UserTable() {
  if(initialised_) {
  for(unsigned int i = 0; i < std::thread::hardware_concurrency(); i++)
    delete connections_.pop();
  }
}
