#include "server.hpp"
#include <mutex>
#include <sstream>
#include "util/URLParser.hpp"
#include "debug/debug.hpp"

using namespace clas_digital;


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

  nlohmann::json cred;
  cred["email"] = parser["email"];
  cred["password"] = parser["password"];
  // Try to do a login with the obtained data
  auto cookie = users_->LogIn(std::move(cred));

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
  auto usr = GetUserFromCookie(req.get_header_value("Cookie"));
  if(usr && usr->DoAccessCheck(req.path))
    resp.set_content(users_->GetAsJSON().dump(),"application/json");
  else
    resp.status = 403;
}

void CLASServer::UpdateUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Update the user list based on the json commands received
  auto usr = GetUserFromCookie(req.get_header_value("Cookie"));

  // Check if the user has the required access
  if(!usr || !usr->DoAccessCheck(req.path))
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
          users_->RemoveUser(it);
	      }
	      else if(it["action"]=="CREATE")
	      {
		      //Create the user with the specified email password and access
          users_->AddUser(it);
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


std::shared_ptr<IUser> CLASServer::GetUserFromCookie(const std::string &cookie_ptr)
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

  return users_->GetUserFromCookie(cookie);
}


debug::Error<CLASServer::ReturnCodes> CLASServer::Start(std::string listenAddress)
{
  if(!initialised_)
    return debug::Error(ReturnCodes::ERR_SERVER_NOT_INITIALISED,"The server was not initialised yet");

  //Register all the URI Handler.
  server_.Post("/api/v2/server/login",[this](const httplib::Request &req, httplib::Response &resp){this->HandleLogin(req, resp);});

  server_.Get("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->SendUserList(req,resp);});

  server_.Post("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->UpdateUserList(req,resp);});

  for(auto &it : cfg_.mount_points_)
  {
    file_handler_->AddMountPoint(it);
  }

  file_handler_->AddAlias({"/search","/"},"../web/index.html");

  server_.set_error_handler([this](const auto& req, auto& res) {
        this->file_handler_->ServeFile(req,res);
      });



  event_manager_.TriggerEvent(EventManager::ON_SERVER_START, this, (void*)&server_);
  
  // Check how many times we tried to bind the port
  int port_binding_tries = 0;
  
  //Try to bind the port 3 times always waiting 50 milliseconds in between. If
  //this fails return ERR_PORT_BINDING
  while(!server_.listen(listenAddress.c_str(),cfg_.server_port_))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if(port_binding_tries++ > 3)
      return debug::Error(ReturnCodes::ERR_PORT_BINDING);
  }
  return debug::Error(ReturnCodes::OK);
}


debug::Error<CLASServer::ReturnCodes> CLASServer::InitialiseFromFile(std::filesystem::path config_file, std::filesystem::path user_db_file)
{
  std::ifstream ifs(config_file.string(), std::ios::in);
  if(!ifs.is_open())
    return Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Could not load config file at " + config_file.string());
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  ifs.close();

  return InitialiseFromString(buffer.str(),user_db_file);
}

debug::Error<CLASServer::ReturnCodes> CLASServer::InitialiseFromString(std::string config_file, std::filesystem::path user_db_file)
{
  {
    auto err = cfg_.LoadFromString(config_file);
    if(err)
    {
      return Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Could not load config file.").Next(err);
    }
  }

  debug::log(debug::LOG_DEBUG,"Config loaded, loading plugins now...\n");
  int i = 0;
  for(auto &it : cfg_.plugins_)
  {
    plugin_manager_.LoadPlugin(std::to_string(i++), it, this);
    debug::log(debug::LOG_DEBUG,"Loaded plugin ",it,"with id ",i-1,"\n");
  }

  if(!file_handler_)
  {
    debug::log(debug::LOG_DEBUG,"Detected file cache size of ",cfg_.file_cache_size_," Bytes. Creating the file handler now as none was supplied by the plugins.\n");
    file_handler_ = std::make_shared<FileHandler>(cfg_.file_cache_size_);
  }
  
  if(!users_)
  {
    users_.reset(new UserTable);
    auto err = users_->Load(user_db_file);
    if(err.GetErrorCode() != UserTable::RET_OK)
    {
      debug::log(debug::LOG_ERROR,"Could not create user table in RAM!\n");
      return debug::Error(ReturnCodes::ERR_USERTABLE_INITIALISE,"Could not initialise user table subsytem").Next(err);
    }
    debug::log(debug::LOG_DEBUG,"Created and loaded user table from path ",user_db_file,"\n");
  }


  if(!ref_manager_)
  {
    if(cfg_.reference_manager_ != "zotero")
      return debug::Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Unknown reference manager, was also not loaded by plugin");
    else
    {
      auto ptr = new ZoteroReferenceManager;
      ref_manager_ = std::shared_ptr<IReferenceManager>(ptr);
      auto err2 = ptr->Initialise(cfg_.reference_config_);
      if(err2)
      {
        debug::log(debug::LOG_WARNING,"Could not load reference manager config and no reference manager was supplied by plugins! Proceeding without reference manager.\n");
        ref_manager_.reset();
      }
    }
  }

  initialised_ = true;
  event_manager_.TriggerEvent(EventManager::ON_AFTER_INITIALISE, this, nullptr);

  return Error(ReturnCodes::OK);
}

void CLASServer::Stop()
{
  // Stop the server and tell the status bit about the changed status
  server_.stop();
  event_manager_.TriggerEvent(EventManager::ON_SERVER_STOP, this, (void*)&server_);
}

ServerConfig &CLASServer::GetServerConfig()
{
  return cfg_;
}

EventManager &CLASServer::GetEventManager()
{
  return event_manager_;
}


std::shared_ptr<UserTable> CLASServer::GetUserTable()
{
  return users_;
}


bool CLASServer::IsRunning()
{
  return server_.is_running();
}

CLASServer::CLASServer()
{
  initialised_ = false;
}
