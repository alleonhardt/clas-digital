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
    UserTable();
    bool Load(std::filesystem::path database_path);
    ~UserTable();

  private:
    sqlite3 *user_database_;
};

#endif
