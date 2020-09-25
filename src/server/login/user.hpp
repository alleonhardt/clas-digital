#ifndef CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#define CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#include <queue>
#include <thread>
#include <shared_mutex>
#include <map>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <debug/debug.hpp>


namespace clas_digital
{
  /**
   * @brief The basic user class, this contains the email address and the access
   * of an user. At most times the Server won't need more to grant access to
   * ressources etc.
   */
  class IUser
  {
    public:
      enum ExportPurpose
      {
        EXPORT_FOR_SAVING_TO_FILE,
        EXPORT_FOR_SENDING_TO_ADMIN
      };

      enum ConstructFlags
      {
        CONSTRUCT_NEW_USER,
        CONSTRUCT_LOAD_FROM_FILE
      };

      /**
       * @brief Creates a new user from an email and the access rights. Every user
       * must have an email and access rights.
       *
       * @param email The email from the user
       * @param acc The access rights the user has.
       */
      virtual bool ConstructFromJSON(const nlohmann::json &js, ConstructFlags flags) = 0;
      virtual void UpdateFromJSON(const nlohmann::json &js) = 0;
      virtual std::string GetPrimaryKey() = 0;
      virtual bool CheckCredentials(nlohmann::json options) = 0;
      virtual nlohmann::json ExportToJSON(ExportPurpose purpose) = 0;
      virtual int GetAccess() = 0;
  };


  class User : public IUser
  {
    public:
      User();
      User(std::string email);
      bool ConstructFromJSON(const nlohmann::json &js,ConstructFlags flags) override;
      std::string GetPrimaryKey() override;
      nlohmann::json ExportToJSON(ExportPurpose purpose) override;
      bool CheckCredentials(nlohmann::json options) override;
      void UpdateFromJSON(const nlohmann::json &js) override;
      int GetAccess() override;

      enum Access
      {
        ACC_READ,
        ACC_WRITE,
        ACC_ADMIN
      };

    private:
      std::string email_;
      std::string password_;
      int access_;
  };

  std::string CreateRandomString(int len);
  std::string sha3_512(const std::string& input);

  class UserTable
  {
    public:
      /**
       * @brief All valid return codes that could be returned by the user table
       * class.
       */
      enum ReturnValues
      {
        ///< Everything went well.
        RET_OK = 0,

        ///< The user one tries to create already exists.
        RET_USER_EXISTS = 1,

        ///< Unknown error. The implementation does not know what caused the error
        RET_UNKNOWN_ERROR = 2,

        RET_CANT_OPEN_USER_FILE = 3,

        RET_USER_FILE_CORRUPTED = 4,

        RET_USER_DOES_NOT_EXIST,

        RET_MISSING_PRIMARY_KEY
      };

      /**
       * @brief Create the database in memory, very fast and very important for
       * testing.
       *
       * @return ReturnValues::OK if everything went well. else check the other
       * return codes
       */
      virtual debug::Error<ReturnValues> Load();


      /**
       * @brief Loads the database from the given filesystem path.
       *
       * @param database_path The path to the saved database.
       *
       * @return ReturnValues::OK if everything went well, else check the other
       * return codes.
       */
      virtual debug::Error<ReturnValues> Load(std::filesystem::path database_path);

      virtual debug::Error<ReturnValues> AddUser(nlohmann::json create_credentials);
      virtual debug::Error<ReturnValues> Update(nlohmann::json update_info);
      virtual debug::Error<ReturnValues> RemoveUser(nlohmann::json remove_info);
      virtual debug::Error<ReturnValues> RemoveUserByKey(std::string primary_key);
      virtual std::shared_ptr<IUser> GetUserFromPrimaryKey(std::string key);

      virtual std::string LogIn(nlohmann::json credentials);


      virtual std::shared_ptr<IUser> GetUserFromCookie(const std::string &cookie);
      virtual void RemoveCookie(const std::string &cookie);

      virtual int GetNumUsers() const;
      virtual nlohmann::json GetAsJSON();
      
      virtual void SetCreateUserCallback(std::function<IUser*()> fnc);
      virtual void SetPrimaryKeyFieldName(std::string primary_key_field_name);
      
      virtual debug::Error<ReturnValues> SaveUserTable();

      UserTable();
      ~UserTable();

    private:
      std::shared_mutex users_lock_;
      std::shared_mutex loggedin_lock_;

      std::filesystem::path save_file_;
      
      std::function<IUser*()> create_user_;
      std::string primary_key_field;

      std::map<std::string, std::shared_ptr<IUser>> users_;
      std::map<std::string, std::shared_ptr<IUser>> logged_in_;

      debug::Error<UserTable::ReturnValues> __AddUser(nlohmann::json create_credentials, User::ConstructFlags flags);
  };
};

#endif
