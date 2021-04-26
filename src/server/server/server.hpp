#ifndef CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
#define CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
// Overwrite default thread pool count

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#endif


#include <algorithm>
#include <mutex>
#include <bitset>
#include "login/user.hpp"
#include "server/server_config.hpp"
#include "plugins/EventManager.hpp"
#include "plugins/PlugInManager.hpp"
#include "reference_management/IReferenceManager.h"
#include "zotero/zotero.hpp"
#include "filehandler/filehandler.hpp"
#include "corpus_manager/corpus_manager.h"


namespace clas_digital
{

  /**
   * @brief The main server class, starts the http server and registers all uri
   * handlers, keeps track of the user table and the global configuration.
   */
  class CLASServer
  {
    public:
      /**
       * @brief The ReturnCodes returned by a CLASServer Function.
       */
      enum class ReturnCodes {
        ///< Everything good. Return OK
        OK = 0,

        ///< The usertable returned an error while initialising
        ERR_USERTABLE_INITIALISE = 1,

        ///< The port specified is still in use or could not be accessed with the
        //current access rights.
        ERR_PORT_BINDING = 2,

        ERR_CONFIG_FILE_INITIALISE = 3,

        ERR_SERVER_NOT_INITIALISED = 4
      };


      debug::Error<ReturnCodes> InitialiseFromFile(std::filesystem::path config_file, std::filesystem::path user_db_file);

      debug::Error<ReturnCodes> InitialiseFromString(std::string config_file, std::filesystem::path user_db_file);

      /**
       * @brief Initialises the server starts the server on the specified port and
       * listen for incoming connections
       *
       * @param listenAddress The address to listen for "0.0.0.0" for Internet and
       * local "localhost" for only local connections
       */
      debug::Error<ReturnCodes> Start(std::string listenAddress);


      /**
       * @brief Stops the server and sets the Server started flag to false
       */
      void Stop();



      /**
       * Handles the upload of one file to the remote server. It will accept PDF, JPG and PNG at the moment.
       */
      void do_upload(const httplib::Request& req, httplib::Response &resp);

      /**
       * Create the bibliography given a set of items in the request query.
       */
      void do_create_bibliography(const httplib::Request &req,httplib::Response &resp);

      /**
       * Handle the login of a single user
       */
      void HandleLogin(const httplib::Request& req, httplib::Response &resp);
      void SendUserList(const httplib::Request& req, httplib::Response &resp);
      void UpdateUserList(const httplib::Request& req, httplib::Response &resp);
      void GetMetadata(const httplib::Request& req, httplib::Response &resp);
      void CreateBibliography(const httplib::Request& req, httplib::Response &resp);

      bool IsRunning();

      /**
       * @brief Helper function to extract the cookie from a request.
       *
       * @param cookie The Cookie header like it is set by popular browsers
       * Chrome, Firefox etc.
       *
       * @return The pointer to the user if there is a user associated with the
       * cookie, otherwise nullptr.
       */
      std::shared_ptr<IUser> GetUserFromCookie(const std::string &cookie);

      std::shared_ptr<ServerConfig> &GetServerConfig();

      std::shared_ptr<EventManager> &GetEventManager();
      std::shared_ptr<UserTable> &GetUserTable();
      std::shared_ptr<IFileHandler> &GetFileHandler();
      
      httplib::Server &GetHTTPServer();
      
      std::shared_ptr<IReferenceManager> &GetReferenceManager();

      std::shared_ptr<CorpusManager> &GetCorpusManager();

      void SetAccessFunction(std::function<bool(const httplib::Request&,IUser*)> &&func);


      ~CLASServer();
      static CLASServer &GetInstance();
  
    private:
      ///< The http server forwards all requests to handlers
      httplib::Server server_;

      CLASServer();

      bool initialised_;

      ///< The mutex synchronises atomic actions on the status
      std::mutex exclusive_section_;

      ///< The User table load the users from disk and handles
      //login/create/delete/change user requests
      std::shared_ptr<UserTable> users_;
      std::shared_ptr<IReferenceManager> ref_manager_;
      std::shared_ptr<IFileHandler> file_handler_;
      std::shared_ptr<PlugInManager> plugin_manager_;
      std::shared_ptr<EventManager> event_manager_;
      std::shared_ptr<CorpusManager> corpus_manager_;
      std::shared_ptr<ServerConfig> cfg_;

      std::function<bool(const httplib::Request&,IUser*)> access_func_;
  };

}

#endif
