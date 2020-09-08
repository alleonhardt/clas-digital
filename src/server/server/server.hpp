#ifndef CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
#define CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
// Overwrite default thread pool count
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include <httplib.h>
#include <algorithm>
#include <mutex>
#include <bitset>
#include "login/user.hpp"
#include "server/server_config.hpp"
#include "plugins/EventManager.hpp"

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

    
    /**
     * @brief Returns the Singleton instance of the CLASServer
     *
     * @return A reference to the singleton instance of CLASServer
     */
    static CLASServer &GetInstance();


    debug::Error<ReturnCodes> InitialiseFromFile(std::filesystem::path config_file, std::filesystem::path user_db_file);

    debug::Error<ReturnCodes> InitialiseFromString(std::string config_file, std::filesystem::path user_db_file);
    
    /**
     * @brief Initialises the server starts the server on the specified port and
     * listen for incoming connections
     *
     * @param listenAddress The address to listen for "0.0.0.0" for Internet and
     * local "localhost" for only local connections
     * @param startPort The port to start the server on
     */
    debug::Error<ReturnCodes> Start(std::string listenAddress);


    /**
     * @brief Stops the server and sets the Server started flag to false
     */
    void Stop();
    


    void HandleLogin(const httplib::Request& req, httplib::Response &resp);
    void SendUserList(const httplib::Request& req, httplib::Response &resp);
    void UpdateUserList(const httplib::Request& req, httplib::Response &resp);

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
    const User *GetUserFromCookie(const std::string &cookie);

    ServerConfig &GetServerConfig();
    cl_events::EventManager &GetEventManager();



  private:
    ///< The http server forwards all requests to handlers
    httplib::Server server_;

    
    bool initialised_;

    ///< The mutex synchronises atomic actions on the status
    std::mutex exclusive_section_;

    ///< The User table load the users from disk and handles
    //login/create/delete/change user requests
    UserTable users_;

    ServerConfig cfg_;
    cl_events::EventManager event_manager_;

    
    /**
     * @brief Make the constructor private to prevent the user from
     * creating additional instances of CLASServer. The CLASServer is a
     * singleton class.
     */
    CLASServer();
};


#endif
