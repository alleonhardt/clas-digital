#include "server.hpp"
#include <mutex>
#include "util/URLParser.hpp"
#include "debug/debug.hpp"


CLASServer &CLASServer::GetInstance()
{
  //Returns the only valid instance of the server
  static CLASServer server;
  return server;
}

void CLASServer::HandleLogin(const httplib::Request &req, httplib::Response &resp)
{
  //First parse the url encoded body of the post request
  URLParser parser(req.body);

  // Try to do a login with the obtained data
  auto cookie = users_.LogIn(parser["email"], parser["password"]);

  //No cookie created deny access
  if(cookie == "") {
    resp.status = 403;
  }
  else {
    //Create the Set Cookie id to make the browser set the cookie on receiving
    //this.
    std::string set_cookie = "SESSID=" + cookie;
    set_cookie += "; SECURE";
    resp.status = 200;
    resp.set_header("Set-Cookie", std::move(set_cookie));
  }
}

void CLASServer::SendUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Send back the user list if the user is an admin user, first check if there
  //is an user associated with the cookie and if it is. Check if the user is an
  //admin user
  const User *usr = GetUserFromCookie(req.get_header_value("Cookie"));
  if(!usr || usr->Access() != UserAccess::ADMIN)
    resp.status = 403;
  else
    resp.set_content(users_.GetAsJSON().dump(),"application/json");
}

void CLASServer::UpdateUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Update the user list based on the json commands received
  const User *usr = GetUserFromCookie(req.get_header_value("Cookie"));

  // Check if the user has the required access
  if(!usr || usr->Access() != UserAccess::ADMIN)
    resp.status = 403;
  else
  {
    try
    {
      auto js = nlohmann::json::parse(req.body);
      //Iterate through the given commands and execute them based on the action
      //they provided
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
      //Something went wrong tell the callee about this
      resp.status = 400;
    }
  }
}


const User *CLASServer::GetUserFromCookie(const std::string &cookie_ptr)
{
  //Try to extract the SESSID from the cookie field
  //If the cookie field is empty return 0 as there is no way an user is logged
  //in without a cookie
  if(cookie_ptr=="") return nullptr;

  //Find the SESSID in the cookie string. A cookie string looks like: 
  //Cookie: BREAD=123jasdklasd; SESSID=Whatever
  //First try to find the start of the cookie used by our authentification
  //system then determine the length of the cookie.
  auto pos = cookie_ptr.find("SESSID=");
  auto pos2 = cookie_ptr.find(";",pos);
  std::string cookie = "";

  // Extract the cookie from the string based on start and end of the cookie
  if(pos2==std::string::npos)
	  cookie = cookie_ptr.substr(pos+7);
  else
	  cookie = cookie_ptr.substr(pos+7,pos2-(pos+7));

  return users_.GetUserFromCookie(cookie);
}


CLASServer::ReturnCodes CLASServer::Start(std::string listenAddress, int startPort)
{
  // Try to load the user table, if this fails notify the user and stop
  // initialising
  if(users_.Load() != UserTable::ReturnCodes::OK) 
  {
    debug::log(debug::LOG_ERROR,"Could not create user table in RAM!\n");
    return ReturnCodes::ERR_USERTABLE_INITIALISE;
  }

  //Register all the URI Handler.
  server_.Post("/api/v2/server/login",[this](const httplib::Request &req, httplib::Response &resp){this->HandleLogin(req, resp);});

  server_.Get("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->SendUserList(req,resp);});

  server_.Post("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->UpdateUserList(req,resp);});


  // Check how many times we tried to bind the port
  int port_binding_tries = 0;
  
  // Set the server started bit to true
  Status(StatusBits::SERVER_STARTED,true);

  //Try to bind the port 3 times always waiting 50 milliseconds in between. If
  //this fails return ERR_PORT_BINDING
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
  // Set the bit to the new values, the lock guard
  // guarantees thread safety for this operation
  std::lock_guard lck(exclusive_section_);
  status_.set((int)bit,value);
}

bool CLASServer::Status(StatusBits bit)
{
  //Return the status for a given bit. the lock guard
  //ensures thread safety for this operation
  std::lock_guard lck(exclusive_section_);
  return status_[(int)bit];
}

void CLASServer::Stop()
{
  // Stop the server and tell the status bit about the changed status
  server_.stop();
  Status(StatusBits::SERVER_STARTED,false);
}


bool CLASServer::IsRunning()
{
  return server_.is_running();
}

CLASServer::CLASServer()
{
  //Reset all bits in the status bit field
  status_.reset();
  
  // Use this as the standard port to open the server on.
  startPort_ = 1409;
}
