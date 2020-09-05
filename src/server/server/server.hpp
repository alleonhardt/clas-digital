#ifndef CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
#define CLASDIGITAL_SRC_SERVER_SERVER_SERVER_HPP
// Overwrite default thread pool count
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include <httplib.h>
#include <algorithm>
#include <mutex>
#include <bitset>
#include "login/user.hpp"

class CLASServer
{
  public:
    enum class ReturnCodes {
      OK = 0,
      ERR_USERTABLE_INITIALISE = 1,
      ERR_PORT_BINDING = 2
    };
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
    void Stop();
    

    enum class StatusBits
    {
      SERVER_STARTED = 1,
      GLOBAL_SHUTDOWN = 2
    };

    void Status(StatusBits bit, bool value);
    bool Status(StatusBits bit);


    void HandleLogin(const httplib::Request& req, httplib::Response &resp);
    void SendUserList(const httplib::Request& req, httplib::Response &resp);
    void UpdateUserList(const httplib::Request& req, httplib::Response &resp);
    const User *GetUserFromCookie(const std::string &cookie);



  private:
    httplib::Server server_;
    int startPort_;
    std::bitset<32> status_;
    std::mutex exclusive_section_;
    UserTable users_;

    CLASServer();
};


#endif
