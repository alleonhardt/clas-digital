#ifndef CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
#define CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
// Overwrite default thread pool count
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include <httplib.h>
#include <algorithm>
#include <mutex>
#include <bitset>
#include "login/user.hpp"

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
      ERR_PORT_BINDING = 2
    };

    
    /**
     * @brief Returns the Singleton instance of the CLASServer
     *
     * @return A reference to the singleton instance of CLASServer
     */
    static CLASServer &GetInstance();

    
    /**
     * @brief Initialises the server starts the server on the specified port and
     * listen for incoming connections
     *
     * @param listenAddress The address to listen for "0.0.0.0" for Internet and
     * local "localhost" for only local connections
     * @param startPort The port to start the server on
     */
    ReturnCodes Start(std::string listenAddress, int startPort);


    /**
     * @brief Stops the server and sets the Server started flag to false
     */
    void Stop();
    

    /**
     * @brief The current server status describes the current state of the
     * server
     */
    enum class StatusBits
    {
      ///< The server has been started and is currently listening for new
      //connections
      SERVER_STARTED = 1,

      ///< The global shutdown bit indiciates if the server is trying to shut
      //down at the moment
      GLOBAL_SHUTDOWN = 2
    };

    /**
     * @brief Sets the status bit to the given value.
     *
     * @param bit The bit to set to a new value
     * @param value The value to set the bit to
     */
    void Status(StatusBits bit, bool value);


    /**
     * @brief Returns the current value of the bit back to the caller
     *
     * @param bit The bit to ask about
     *
     * @return The value of the specified bit
     */
    bool Status(StatusBits bit);


    void HandleLogin(const httplib::Request& req, httplib::Response &resp);
    void SendUserList(const httplib::Request& req, httplib::Response &resp);
    void UpdateUserList(const httplib::Request& req, httplib::Response &resp);


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



  private:
    ///< The http server forwards all requests to handlers
    httplib::Server server_;

    ///< The port to start the server on
    int startPort_;
    
    ///< Contains all current and future used bits.
    std::bitset<32> status_;

    ///< The mutex synchronises atomic actions on the status
    std::mutex exclusive_section_;

    ///< The User table load the users from disk and handles
    //login/create/delete/change user requests
    UserTable users_;

    
    /**
     * @brief Make the constructor private to prevent the user from
     * creating additional instances of CLASServer. The CLASServer is a
     * singleton class.
     */
    CLASServer();
};


#endif
