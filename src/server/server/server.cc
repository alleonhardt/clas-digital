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

void CLASServer::SendUserList(const httplib::Request &req, httplib::Response &resp)
{
  const User *usr = GetUserFromCookie(req.get_header_value("Cookie"));
  if(!usr || usr->Access() != UserAccess::ADMIN)
    resp.status = 403;
  else
    resp.set_content(users_.GetAsJSON().dump(),"application/json");
}

void CLASServer::UpdateUserList(const httplib::Request &req, httplib::Response &resp)
{
  const User *usr = GetUserFromCookie(req.get_header_value("Cookie"));
  if(!usr || usr->Access() != UserAccess::ADMIN)
    resp.status = 403;
  else
  {
    try
    {
      auto js = nlohmann::json::parse(req.body);
      for(auto &it : js)
      {
        if(it["action"]=="DELETE")
	      {
		      //Remove the user if the action is delete if one of the necessary variables does not exist then throw an error and return an error not found
          users_.RemoveUser(it["email"]);
	      }
	      else if(it["action"]=="CREATE")
	      {
		      //Create the user with the specified email password and access
          users_.AddUser(it["email"], it["password"], "Unknown", it["access"]);
	      }
      }
    }
    catch(...)
    {
      resp.status = 400;
    }
  }
}


const User *CLASServer::GetUserFromCookie(const std::string &cookie_ptr)
{
  if(cookie_ptr=="") return nullptr;

  auto pos = cookie_ptr.find("SESSID=");
  auto pos2 = cookie_ptr.find(";",pos);
  std::string cookie = "";

  if(pos2==std::string::npos)
	  cookie = cookie_ptr.substr(pos+7);
  else
	  cookie = cookie_ptr.substr(pos+7,pos2-(pos+7));

  return users_.GetUserFromCookie(cookie);
}


CLASServer::ReturnCodes CLASServer::Start(std::string listenAddress, int startPort)
{
  if(users_.Load() != UserTable::ReturnCodes::OK) 
  {
    debug::log(debug::LOG_ERROR,"Could not create user table in RAM!\n");
    return ReturnCodes::ERR_USERTABLE_INITIALISE;
  }

  Status(StatusBits::SERVER_STARTED,true);
  server_.Post("/api/v2/server/login",[this](const httplib::Request &req, httplib::Response &resp){this->HandleLogin(req, resp);});

  server_.Get("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->SendUserList(req,resp);});

  server_.Post("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->UpdateUserList(req,resp);});


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
