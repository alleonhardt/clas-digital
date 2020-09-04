#ifndef CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#define CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#include <sqlite3.h>
#include <filesystem>

enum class UserAccess
{
  READ = 1,
  WRITE = 2,
  ADMIN = 4
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

    UserTable();
    ReturnCodes Load(std::filesystem::path database_path);
    ReturnCodes AddUser(std::string email, std::string password, std::string name, UserAccess acc);
    ~UserTable();

  private:
    sqlite3 *user_database_;
};

#endif
