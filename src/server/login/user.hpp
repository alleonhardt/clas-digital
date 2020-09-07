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
#include <nlohmann/json.hpp>
#include <debug/debug.hpp>

/**
 * @brief Describes the different level of user access
 */
enum class UserAccess
{
  ///< The user is allowed to read all copyright protected books
  READ = 1,

  ///< The user is allowed to upload new books and make changes to existing
  //books
  WRITE = 2,

  ///< The user is allowed to do all that the READ/WRITE users can do but
  //additional grants user creating abilitys and enables one to restart the
  //server.
  ADMIN = 4
};

/**
 * @brief The basic user class, this contains the email address and the access
 * of an user. At most times the Server won't need more to grant access to
 * ressources etc.
 */
class User
{
  public:
    /**
     * @brief Creates a new user from an email and the access rights. Every user
     * must have an email and access rights.
     *
     * @param email The email from the user
     * @param acc The access rights the user has.
     */
    User(std::string email, UserAccess acc);

    /**
     * @brief The getter method for the email address.
     *
     * @return Returns the email address of the user.
     */
    const std::string &Email() const;

    /**
     * @brief The getter method for the access rights.
     *
     * @return Returns the current access rights, that the user posseses
     */
    UserAccess Access() const;

  private:
    ///< The email of the user, is also primary key in the SQL table
    std::string email_;

    ///< The access rights the user has.
    UserAccess access_;
};

class UserTable
{
  public:
    /**
     * @brief All valid return codes that could be returned by the user table
     * class.
     */
    enum class ReturnCodes
    {
      ///< Everything went well.
      OK = 0,

      ///< The user one tries to create already exists.
      USER_EXISTS = 1,

      ///< Unknown error. The implementation does not know what caused the error
      UNKNOWN_ERROR = 2,
    };

    /**
     * @brief Create the database in memory, very fast and very important for
     * testing.
     *
     * @return ReturnCodes::OK if everything went well. else check the other
     * return codes
     */
    debug::Error<ReturnCodes> Load();


    /**
     * @brief Loads the database from the given filesystem path.
     *
     * @param database_path The path to the saved database.
     *
     * @return ReturnCodes::OK if everything went well, else check the other
     * return codes.
     */
    debug::Error<ReturnCodes> Load(std::filesystem::path database_path);
  
    debug::Error<ReturnCodes> AddUser(std::string email, std::string password, std::string name, UserAccess acc);
    debug::Error<ReturnCodes> RemoveUser(std::string email);
    
    std::string LogIn(std::string email, std::string password);
    User *GetUserFromCookie(const std::string &cookie);
    void RemoveCookie(const std::string &cookie);
    int GetNumUsers();
    nlohmann::json GetAsJSON();

    ~UserTable();
    UserTable();

  private:
    SQLite::Database* connection_;
    std::mutex class_lock_;
    
    std::map<std::string, User> logged_in_;
};

#endif
