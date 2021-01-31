#include "server.hpp"
#include <mutex>
#include <sstream>
#include <csignal>
#include <thread>
#include "filehandler/filehandler.hpp"
#include "plugins/EventManager.hpp"
#include "plugins/PlugInManager.hpp"
#include "server/server_config.hpp"
#include "util/URLParser.hpp"
#include "debug/debug.hpp"
#include "zotero/zotero.hpp"

using namespace clas_digital;

std::atomic<bool> gCaughtUserAbort = false;

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
  if(access_func_(req,GetUserFromCookie(req.get_header_value("cookie")).get()))
    resp.set_content(users_->GetAsJSON().dump(),"application/json");
  else
    resp.status = 403;
}

void CLASServer::UpdateUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Update the user list based on the json commands received
  auto usr = GetUserFromCookie(req.get_header_value("Cookie"));

  // Check if the user has the required access
  if(!access_func_(req,GetUserFromCookie(req.get_header_value("cookie")).get()))
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

void CLASServer::GetMetadata(const httplib::Request& req, httplib::Response &resp)
{
  if(!access_func_(req,GetUserFromCookie(req.get_header_value("cookie")).get()))
  {
    resp.status = 403;
    return;
  }

  auto &items = corpus_manager_->item_references();
  auto find = items->find(req.get_header_value("key"));
  if(find!=items->end())
  {
    resp.set_content(find->second->json().dump(),"application/json");
  }
  else
    resp.status = 404;
}

void CLASServer::CreateBibliography(const httplib::Request& req, httplib::Response &resp)
{
  resp.status = 404;
}



debug::Error<CLASServer::ReturnCodes> CLASServer::Start(std::string listenAddress)
{
  if(!initialised_)
    return debug::Error(ReturnCodes::ERR_SERVER_NOT_INITIALISED,"The server was not initialised yet");

  corpus_manager_->UpdateZotero(ref_manager_.get(),file_handler_.get());

  //Register all the URI Handler.
  server_.Post("/api/v2/server/login",[this](const httplib::Request &req, httplib::Response &resp){this->HandleLogin(req, resp);});

  server_.Get("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->SendUserList(req,resp);});

  server_.Post("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->UpdateUserList(req,resp);});

  for(auto &it : cfg_->mount_points_)
  {
    file_handler_->AddMountPoint(it);
  }


  server_.set_error_handler([this](const auto& req, auto& res) {
        this->file_handler_->ServeFile(req,res);
      });



  event_manager_->TriggerEvent(EventManager::ON_SERVER_START, (void*)&server_);
  
  // Check how many times we tried to bind the port
  int port_binding_tries = 0;
  
  //Try to bind the port 3 times always waiting 50 milliseconds in between. If
  //this fails return ERR_PORT_BINDING
  while(!server_.listen(listenAddress.c_str(),cfg_->server_port_))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if(port_binding_tries++ > 3)
      return debug::Error(ReturnCodes::ERR_PORT_BINDING);
  }
  debug::log(debug::LOG_DEBUG,"Exiting server loop now!\n");
  
  return debug::Error(ReturnCodes::OK);
}


debug::Error<CLASServer::ReturnCodes> CLASServer::InitialiseFromFile(std::filesystem::path config_file, std::filesystem::path user_db_file)
{
  std::ifstream ifs(config_file.string(), std::ios::in);
  if(!ifs.is_open())
    return Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Could not load config file at " +  config_file.string());
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  ifs.close();

  return InitialiseFromString(buffer.str(),user_db_file);
}

debug::Error<CLASServer::ReturnCodes> CLASServer::InitialiseFromString(std::string config_file, std::filesystem::path user_db_file)
{
  auto err = cfg_->LoadFromString(config_file);
  if(err)
  {
    return Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Could not load config file.").Next(err);
  }

  debug::log(debug::LOG_DEBUG,"Config loaded, loading plugins now...\n");
  int i = 0;
  for(auto &it : cfg_->plugins_)
  {
    plugin_manager_->LoadPlugin(std::to_string(i++), it, this);
    debug::log(debug::LOG_DEBUG,"Loaded plugin ",it,"with id ",i-1,"\n");
  }

  if(!file_handler_)
  {
    debug::log(debug::LOG_DEBUG,"Detected file cache size of ",cfg_->file_cache_size_," Bytes. Creating the file handler now as none was supplied by the plugins.\n");
    file_handler_ = std::make_shared<FileHandler>(cfg_->file_cache_size_);
  }
  
  if(!users_)
  {
    users_.reset(new UserTable(event_manager_.get()));
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
    if(cfg_->reference_manager_ != "zotero")
      return debug::Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Unknown reference manager, was also not loaded by plugin\n.");
    else
    {
      auto ptr = new ZoteroReferenceManager(event_manager_.get());
      ref_manager_ = std::shared_ptr<IReferenceManager>(ptr);
      auto err2 = ptr->Initialise(cfg_->reference_config_);
      if(err2)
      {
        debug::log(debug::LOG_WARNING,"Could not load reference manager config and no reference manager was supplied by plugins! Proceeding without reference manager.\n");
        ref_manager_.reset();
      }
    }
  }

  if(!corpus_manager_)
    corpus_manager_ = std::make_shared<CorpusManager>();

  initialised_ = true;
  debug::log(debug::LOG_DEBUG,"Initialise done, triggering ON_AFTER_INITIALISE now!\n");
  event_manager_->TriggerEvent(EventManager::ON_AFTER_INITIALISE, nullptr);

  return Error(ReturnCodes::OK);
}

void CLASServer::Stop()
{
  server_.stop();
  debug::log(debug::LOG_WARNING,"Received shutdown signal. Shutting down server, triggering ON_SERVER_STOP!\n");
  event_manager_->TriggerEvent(EventManager::ON_SERVER_STOP, (void*)&server_);
}

std::shared_ptr<ServerConfig> &CLASServer::GetServerConfig()
{
  return cfg_;
}

std::shared_ptr<EventManager> &CLASServer::GetEventManager()
{
  return event_manager_;
}


std::shared_ptr<UserTable> &CLASServer::GetUserTable()
{
  return users_;
}


std::shared_ptr<IFileHandler> &CLASServer::GetFileHandler()
{
  return file_handler_;
}
      
httplib::Server &CLASServer::GetHTTPServer()
{
  return server_;
}

std::shared_ptr<IReferenceManager> &CLASServer::GetReferenceManager()
{
  return ref_manager_;
}

std::shared_ptr<CorpusManager> &CLASServer::GetCorpusManager()
{
  return corpus_manager_;
}

void CLASServer::SetAccessFunction(std::function<bool(const httplib::Request&,IUser*)> &&func)
{
  access_func_ = func;
}



bool CLASServer::IsRunning()
{
  return server_.is_running();
}

void signal_handler(int sig)
{
  std::thread([](){
  std::exit(0);
  }).detach();
}

CLASServer::CLASServer()
{
  std::signal(SIGINT,signal_handler);
  std::signal(SIGTERM, signal_handler);
  initialised_ = false;

  access_func_ = [](const httplib::Request&,IUser*) {
    return true;
  };
  
  cfg_ = std::make_shared<ServerConfig>();
  plugin_manager_ = std::make_shared<PlugInManager>();
  event_manager_ = std::make_shared<EventManager>(this);
}

CLASServer::~CLASServer()
{
  Stop();
}

CLASServer &CLASServer::GetInstance()
{
  static CLASServer clas;
  return clas;
}
