#ifndef CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#define CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <queue>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include <filesystem>

enum class UserAccess
{
  READ = 1,
  WRITE = 2,
  ADMIN = 4
};

class User
{
  public:
    User(std::string email, UserAccess acc);
    std::string Email();
    UserAccess Access();

  private:
    std::string email_;
    UserAccess access_;
};

class UserTable
{
  public:
    enum class ReturnCodes
    {
      OK = 1,
      USER_EXISTS = 2,
      UNKNOWN_ERROR = 3,
    };

    ReturnCodes Load(std::filesystem::path database_path);
    ReturnCodes AddUser(std::string email, std::string password, std::string name, UserAccess acc);
    ReturnCodes RemoveUser(std::string email);
    
    std::string LogIn(std::string email, std::string password);
    User *GetUserFromCookie(const std::string &cookie);
    void RemoveCookie(const std::string &cookie);
    int GetNumUsers();

    ~UserTable();
    UserTable();

  private:
    SQLite::Database* connection_;
    std::mutex class_lock_;
    
    std::map<std::string, User> logged_in_;
};

#endif
