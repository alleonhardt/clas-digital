#include "filehandler.hpp"
#include "filehandler/util.h"
#include "debug/debug.hpp"
debug::LogLevel debug::LogClass::gLogLevel;
using namespace clas_digital;

FileHandler::FileHandler(long long size) : cache_(size)
{
  file_types_[".css"] = "text/css";
  file_types_[".js"] = "application/javascript";
  file_types_[".html"] = "text/html";
  file_types_[".txt"] = "text/plain";
  file_types_[".htm"] = "text/html";
  file_types_[".jpg"] = "image/jpeg";
  file_types_[".svg"] = "image/svg+xml";
  file_types_[".jpeg"] = "image/jpg";
  file_types_[".pdf"] = "application/pdf";
  file_types_[".png"] = "application/png";
  file_types_[".json"] = "application/json";
  file_types_[".ico"] = "image/x-icon";

  cache_file_callback_ = [](const std::filesystem::path &p){
    return true;
  };
}

std::string FileHandler::__getFileMimetype(const std::filesystem::path &p)
{
  std::string file_type = "text/html";
  auto it = file_types_.find(p.extension().string());
  if(it!=file_types_.end())
    file_type = it->second;
  return file_type;
}


void FileHandler::AddMountPoint(std::filesystem::path pt)
{
  mount_points_.push_back(pt);
}

void FileHandler::AddAlias(std::vector<std::string> from, std::filesystem::path to)
{
  std::shared_ptr<UnmutableCacheableFile> pt(new UnmutableCacheableFile(to));
  for(auto &it : from)
    cache_.insert(it,pt);
}
      
void FileHandler::ServeFile(const httplib::Request &req, httplib::Response &resp, bool abortoncachemiss)
{
  if(req.path.find("..") != -1 || req.path.find("~") != -1)
  {
    resp.status = 403;
    return;
  }



  cache_.load(req.path,
      [&resp,&req,this](void* data,long long size)
      {
        resp.status = 200;
        resp.set_content((const char*)data,size,__getFileMimetype(req.path).c_str());
      },
      [&req,this,&resp,abortoncachemiss](int err)
      {
        if(abortoncachemiss) return;


        for(auto &it : mount_points_)
        {
          std::filesystem::path pt = it.string()+req.path;
          if(std::filesystem::is_directory(pt))
            pt=pt/"index.html";

          if(std::filesystem::exists(pt))
          {
            cache_.insert(req.path,std::make_unique<UnmutableCacheableFile>(pt));
            ServeFile(req,resp,true);
          }
        }
      });
}
