#include "server.hpp"
#include <mutex>
#include <sstream>
#include <csignal>
#include <thread>
#include <sstream>
#include <filesystem>
#include "filehandler/filehandler.hpp"
#include "plugins/EventManager.hpp"
#include "plugins/PlugInManager.hpp"
#include "server/server_config.hpp"
#include "util/URLParser.hpp"
#include "debug/debug.hpp"
#include "zotero/zotero.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
  std::cout<<cred<<std::endl;
  auto cookie = users_->LogIn(std::move(cred));

  //No cookie created deny access
  if(cookie == "") {
    resp.status = 403;
  }
  else {
    //Create the Set Cookie id to make the browser set the cookie on receiving
    //this.
    std::string set_cookie = "SESSID=" + cookie;
    set_cookie += ";";
    //SECURE in HTTTPS
    //set_cookie += ";";
    resp.status = 200;
    std::cout<<set_cookie<<std::endl;
    resp.set_header("Set-Cookie", std::move(set_cookie));
  }
}

void CLASServer::SendUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Send back the user list if the user is an admin user, first check if there
  //is an user associated with the cookie and if it is. Check if the user is an
  //admin user
  auto usr = GetUserFromCookie(req.get_header_value("Cookie"));
  if(usr && (usr->GetAccess()&User::Access::ACC_ADMIN == User::Access::ACC_ADMIN))
    resp.set_content(users_->GetAsJSON().dump(),"application/json");
  else
    resp.status = 403;
}

void CLASServer::UpdateUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Update the user list based on the json commands received
  auto usr = GetUserFromCookie(req.get_header_value("Cookie"));

  if(!usr || (usr->GetAccess()&User::Access::ACC_ADMIN != User::Access::ACC_ADMIN))
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
        if(it["action"]=="CHANGEACCESS")
        {
          auto res = users_->GetUserFromPrimaryKey(it["email"]);
          if(res)
            res->SetAccess(it["access"].get<int>());
        }
        else if(it["action"]=="DELETE")
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
  server_.Post("/login",[this](const httplib::Request &req, httplib::Response &resp){this->HandleLogin(req, resp);});

  server_.Get("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->SendUserList(req,resp);});

  server_.Post("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->UpdateUserList(req,resp);});
   server_.Post("/upload",[&](const httplib::Request &req, httplib::Response &resp){do_upload(req,resp);});
   server_.Get("/createbibliography",[this](const httplib::Request &req, httplib::Response &resp) { do_create_bibliography(req,resp);});
 
 
 
   server_.set_error_handler([this](const auto& req, auto& res) {
       auto usr = GetUserFromCookie(req.get_header_value("Cookie"));
       if(!usr) {
        res.set_header("Set-Cookie","SESSID=deleted; expires=Thu, 01 Jan 1970 00:00:00 GMT");
       }
       const std::regex base_regex("/books/([a-z0-9]+)/pages/.*");
       std::smatch base_match; 
       if(std::regex_match(req.path,base_match,base_regex)) {
          std::string key = base_match[1]; 
          try {
            auto book = corpus_manager_->item_references()->at(key);
            if(book->HasCopyright()) {
            if(!usr || (usr->GetAccess()&User::Access::ACC_READ != User::Access::ACC_READ)) {
            res.status = 403;
            return;
           }
          }
          }
          catch(...) {
            res.status = 404;
            return;
          }

       }
       const std::regex base_admin("/private/admin.*");
       const std::regex base_write("/private/write.*");
       if(std::regex_match(req.path,base_admin)) {
         if(!usr || (usr->GetAccess()&User::Access::ACC_ADMIN != User::Access::ACC_ADMIN)) {
            res.status = 307;
            res.set_header("Location","/");
            return;
          }
        }

        if(std::regex_match(req.path,base_write)) {
         if(!usr || (usr->GetAccess()&User::Access::ACC_WRITE != User::Access::ACC_WRITE)) {
            res.status = 307;
            res.set_header("Location","/");
            return;
          }
        }

       this->file_handler_->ServeFile(req,res);
           });

  file_handler_->AddAlias({"/search"},"web/index.html");


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

void CLASServer::do_create_bibliography(const httplib::Request &req,httplib::Response &resp)
{
  try
  {
    std::string inBook = req.get_param_value("books");
    std::string_view inBookView(inBook);
    if(inBook=="")
    {resp.set_content("{}","application/json");return;}

    std::string retval="<html><head><meta charset=\"utf-8\"></head><body>";
    auto &mapBooks = corpus_manager_->item_references();

    size_t x_new = 0;
    size_t x_last = 0;
    for(;;)
    {
      x_new = inBookView.find(",",x_last);
      std::string key;
      if(x_new == std::string::npos)
        key = inBookView.substr(x_last);
      else
        key = inBookView.substr(x_last,x_new-x_last);

      try
      {
        auto &book = mapBooks->at(key);
        retval+= "<p>";
        retval+= book->json()["citation"];
        retval+="</p>";
      }
      catch(...) {}

      if(x_new==std::string::npos) break;
      x_last = x_new+1;
    }
    retval+="</body></html>";
    resp.set_content(retval.c_str(),"text/html");
  }
  catch(...)
  {
    resp.set_content("{}","application/json");return;
  }
}


void CLASServer::do_upload(const httplib::Request& req, httplib::Response &resp)
{
  auto usr = GetUserFromCookie(req.get_header_value("Cookie"));
  if(!usr || (usr->GetAccess()&User::Access::ACC_ADMIN != User::Access::ACC_WRITE)) {
    resp.status = 403;
    return;
  }
  bool forcedWrite=false;
  std::string scanId = "";
  std::string fileName = "";
  std::string ocr_create = "";
  std::string ocr_lang = "";

  try{forcedWrite = req.get_param_value("forced")=="true";}catch(...){};
  try{scanId = req.get_param_value("scanid");}catch(...){};
  try{fileName = req.get_param_value("filename");}catch(...){};
  try{ocr_create = req.get_param_value("create_ocr");}catch(...){};
  try{ocr_lang = req.get_param_value("language");}catch(...){};

  std::cout<<"Authorized request to upload files"<<std::endl;
  std::cout<<"Parsed fileName: "<<fileName<<std::endl;
  std::cout<<"Parsed scanID: "<<scanId<<std::endl;
  std::cout<<"Parsed force Overwrite: "<<forcedWrite<<std::endl;
  if(scanId==""||fileName==""||fileName.find("..")!=std::string::npos||fileName.find("~")!=std::string::npos||scanId.find("..")!=std::string::npos||scanId.find("~")!=std::string::npos)
  {
    resp.status=403;
    resp.set_content("malformed_parameters","text/plain");
    return;
  }
  auto pos = fileName.find_last_of('.');
  std::string fileEnd = fileName.substr(pos+1,std::string::npos);
  std::string fileNameWithoutEnding = fileName.substr(0,pos);

  IReference *book;
  try
  {
    book = corpus_manager_->book(scanId);
  }
  catch(...)
  {
    resp.status=403;
    resp.set_content("book_unknown","text/plain");
    return;
  }

  if(fileEnd == "txt" || fileEnd == "TXT")
    fileName = "ocr.txt";

  auto ret_val = GetFreeSpace(book->GetPath());
  std::cout<<"Free hard disk space on current book path is: "<<ret_val<<std::endl;
  if( ret_val < 2000000000000 ) {
    std::cout<<"Space is too low selecting other upload point"<<std::endl;
    book->MoveTo(file_handler_->GetFreestMountPoint());
  }

  std::string writePath = book->GetPath();
  std::string directory = writePath;

  writePath+="/";
  writePath+=fileName;

  bool doOverwrite = std::filesystem::exists(writePath);
  doOverwrite |= std::filesystem::exists(directory+"/pages/"+fileName);
  if((doOverwrite||!std::filesystem::exists(directory))&&!forcedWrite)
  {
    resp.status=403;
    resp.set_content("file_exists","text/plain");
    return;
  }
  const char *buffer = req.body.c_str();
  auto buffer_length = req.body.length();
  char imgbuffer[4*1014*1024];
  if(fileEnd=="jpg"||fileEnd=="png"||fileEnd=="JPG"||fileEnd=="PNG"||fileEnd=="jp2"||fileEnd=="JP2")
  {
    try
    {
      static std::mutex m;
      std::lock_guard lck(m);
      if(fileEnd=="JP2" || fileEnd == "jp2")
      {
        fileName = fileNameWithoutEnding+".jpg";
        std::string endTmpFileName = "tmp021.jpg";
        bool deleteJP2 = true;
        bool deleteBMP = true;

        std::ofstream ofs("tmp021.jp2",std::ios::out);
        ofs.write(req.body.c_str(),req.body.length());
        ofs.close();

        if(system("opj_decompress -i tmp021.jp2 -o tmp021.bmp") != 0)
        {
          resp.status = 403;
          resp.set_content("Could not convert jp2 to jpg missing openjpeg library!","text/plain");
          return;
        }

        if(system("convert tmp021.bmp tmp021.jpg") == 0)
        {
        }

        if((deleteJP2 && (system("rm tmp021.jp2") != 0)) || (deleteBMP && (system("rm tmp021.bmp") != 0)))
        {
          resp.status = 403;
          resp.set_content("Cleanup error!","text/plain");
          return;
        }
        std::ifstream ifs(endTmpFileName.c_str(),std::ios::in);
        if(!ifs)
        {
          resp.status = 403;
          resp.set_content("Could not convert jp2 to jpg!","text/plain");
          return;
        }
        ifs.read(imgbuffer,4*1024*1024);
        auto len = ifs.gcount();
        ifs.close();
        std::string delname = "rm "+endTmpFileName;
        if(system(delname.c_str()) != 0)
        {
          resp.status = 403;
          resp.set_content("Cleanup error!","text/plain");
          return;
        }

        buffer = imgbuffer;
        buffer_length = len;
      }
      std::string pa = directory;
      pa+="/readerInfo.json";
      nlohmann::json file_desc;
      if(std::filesystem::exists(pa))
      {
        std::ifstream readerFile(pa.c_str(),std::ios::in);
        readerFile>>file_desc;
        readerFile.close();
      }
      else
      {
        file_desc["maxPageNum"] = 0;
        file_desc["pages"] = nlohmann::json::array();
      }

      nlohmann::json entry;
      entry["file"] = fileName;
      std::regex reg("_0*([0-9]+)");
      std::smatch cm;
      int maxPageNum = file_desc["maxPageNum"];
      std::cout<<fileName<<std::endl;
      std::regex_search(fileName,cm,reg);
      std::cout<<"Match size: "<<cm.size()<<std::endl;
      if(cm.size()<2)
      {
        resp.status = 403;
        resp.set_content("Wrong format: Expected [ bsbid_pagenumber.jpg ]","text/plain");
        return;
      }
      std::cout<<"Match: "<<cm[1]<<std::endl;
      entry["pageNumber"]=std::stoi(cm[1]);
      maxPageNum = std::max(maxPageNum,std::stoi(cm[1]));

      int width,height,z;
      stbi_info_from_memory(reinterpret_cast<const unsigned char*>(buffer),buffer_length,&width,&height,&z);
      entry["width"] = width;
      entry["height"] = height;
      bool replace = false;
      for(auto &iter : file_desc["pages"])
      {
        if(iter["pageNumber"]==entry["pageNumber"])
        {
          iter["width"] = width;
          iter["height"] = height;
          iter["file"] = fileName;
          replace=true;
          break;
        }
      }
      file_desc["maxPageNum"] = maxPageNum;
      int what = entry["pageNumber"];
      if(!replace)
        file_desc["pages"].push_back(std::move(entry));

      std::ofstream writer(pa.c_str(),std::ios::out);
      writer<<file_desc;
      writer.close();
    }
    catch(std::exception &e)
    {
      resp.status = 403;
      std::string error = "Error while creating readerInfo.json: ";
      error+=e.what();
      resp.set_content(error.c_str(),"text/plain");
      return;
    }
    writePath = directory;
    writePath += "/pages";
    std::error_code ec;
    if(!std::filesystem::exists(writePath))
      std::filesystem::create_directory(writePath,ec);
    writePath+="/";
    writePath += fileName;
  }
  else if((fileEnd!="txt")&&(fileEnd!="TXT"))
  {
    resp.status = 403;
    resp.set_content("Unsupported file type, the uploader only supports .txt, .jpg and .png files!","text/plain");
    return;
  }
  else
  {
    writePath = directory;
    writePath += "/ocr.txt";
    std::cout<<writePath<<std::endl;
  }
  std::cout<<"Uploading file now! File size: "<<req.body.length()<<std::endl;

  if(doOverwrite)
  {
    static std::mutex ml;
    std::lock_guard lck(ml);
    std::string backupfolder = "web/books/";
    backupfolder += scanId;
    backupfolder+="/backups";
    if(!std::filesystem::exists(backupfolder))
      std::filesystem::create_directory(backupfolder);

    std::string newpath = backupfolder;
    newpath+="/";
    newpath+=fileName;
    if(!std::filesystem::exists(newpath))
      std::filesystem::create_directory(newpath);

    int version = 0;
    for(const auto &dirEntry : std::filesystem::directory_iterator(newpath))
    {
      (void)dirEntry;
      ++version;
    }

    std::string finnewpath = newpath;
    finnewpath+="/";

    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << version;
    finnewpath+=ss.str();
    finnewpath+="some_user";
    finnewpath+="-";
    finnewpath+=fileName;
    std::filesystem::rename(writePath,finnewpath);
  }
  std::cout<<"Writing to "<<writePath<<std::endl;
  std::ofstream ofs(writePath.c_str(),std::ios::out);
  ofs.write(buffer,buffer_length);
  ofs.close();
  resp.status=200;
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

    for(auto &it : cfg_->mount_points_)
    {
      file_handler_->AddMountPoint(it);
    }

    for(auto &it : cfg_->upload_points_) {
      file_handler_->AddUploadPoint(it);
    }
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
      auto ptr = new ZoteroReferenceManager(event_manager_.get(),file_handler_.get());
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
