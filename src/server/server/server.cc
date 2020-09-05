#include "server.hpp"
#include <mutex>
#include "util/URLParser.hpp"



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
    cookie = "SESSID=" + cookie;
    cookie += "; SECURE";
    resp.set_header("Set-Cookie", cookie.c_str());
    resp.status = 200;
  }
}


void CLASServer::Start(std::string listenAddress, int startPort)
{
  Status(StatusBits::SERVER_STARTED,true);
  httplib::Server::Handler handler = std::bind(&CLASServer::HandleLogin, this, std::placeholders::_1,std::placeholders::_2); 
  server_.Post("/api/v2/server/login",std::move(handler));


//  server_.Get("/api/v2/server/get_userlist",&do_senduserlist);
//  server_.Post("/api/v2/server/update_userlist",&do_usertableupdate);


  server_.listen(listenAddress.c_str(),startPort);
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
  users_.Load("users.db");
}
