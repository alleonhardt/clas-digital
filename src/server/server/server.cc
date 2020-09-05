#include "server.hpp"
#include <mutex>
#include "util/URLParser.hpp"
#include "debug/debug.hpp"



CLASServer &CLASServer::GetInstance()
{
  static CLASServer server;
  return server;
}

void CLASServer::HandleLogin(const httplib::Request &req, httplib::Response &resp)
{
  URLParser parser(req.body);
  auto cookie = users_.LogIn(parser["email"], parser["password"]);

  //No cookie created deny access
  if(cookie == "") {
    resp.status = 403;
  }
  else {
    std::string set_cookie = "SESSID=" + cookie;
    set_cookie += "; SECURE";
    resp.status = 200;
    resp.set_header("Set-Cookie", std::move(set_cookie));
  }
}


CLASServer::ReturnCodes CLASServer::Start(std::string listenAddress, int startPort)
{
  if(users_.Load() != UserTable::ReturnCodes::OK) {
    debug::log(debug::LOG_ERROR,"Could not create user table in RAM!\n");
    return ReturnCodes::ERR_USERTABLE_INITIALISE;
  }

  Status(StatusBits::SERVER_STARTED,true);
  server_.Post("/api/v2/server/login",[this](const httplib::Request &req, httplib::Response &resp){this->HandleLogin(req, resp);});


//  server_.Get("/api/v2/server/get_userlist",&do_senduserlist);
//  server_.Post("/api/v2/server/update_userlist",&do_usertableupdate);


  int port_binding_tries = 0;
  while(!server_.listen(listenAddress.c_str(),startPort))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if(port_binding_tries++ > 3)
      return ReturnCodes::ERR_PORT_BINDING;
  }
  return ReturnCodes::OK;
}



void CLASServer::Status(StatusBits bit, bool value)
{
  std::lock_guard lck(exclusive_section_);
  status_.set((int)bit,value);
}

bool CLASServer::Status(StatusBits bit)
{
  std::lock_guard lck(exclusive_section_);
  return status_[(int)bit];
}

void CLASServer::Stop()
{
  server_.stop();
  Status(StatusBits::SERVER_STARTED,false);
}

CLASServer::CLASServer()
{
  status_.reset();
  startPort_ = 1409;
}
