#include "filehandler.hpp"
#include "filehandler/util.h"
using namespace clas_digital;

FileHandler::FileHandler()
{
  file_types_[".css"] = "text/css";
  file_types_[".js"] = "application/javascript";
  file_types_[".html"] = "text/html";
  file_types_[".htm"] = "text/html";
  file_types_[".jpg"] = "image/jpg";
  file_types_[".jpeg"] = "image/jpg";
  file_types_[".pdf"] = "application/pdf";
  file_types_[".png"] = "application/png";
  file_types_[".json"] = "application/json";
  file_types_[".ico"] = "image/x-icon";

  cache_file_callback_ = [](const std::filesystem::path &p){
    auto ext = p.extension();
    if(ext == ".txt" && p.filename() == "ocr.txt")
      return true;
    if(ext == ".html" || ext == ".css" || ext == ".js" || ext == ".json" || ext == ".ico")
      return true;
    return false;
  };
}

void FileHandler::__iterateDirAndCacheFiles(const std::filesystem::path &p)
{
  int indexed_files = 0;
  unsigned long long size = 0;

  for(auto &it : std::filesystem::recursive_directory_iterator(p))
  {
    if(cache_file_callback_(it.path()))
    {
      __cacheFile(it.path());
    }
  }
}

void FileHandler::__cacheFile(const std::filesystem::path &cachePath)
{
  cache_t vec;
  if(!__loadFile(cachePath,vec))
    return;
  __cacheFile(cachePath,vec);
}

void FileHandler::__cacheFile(const std::filesystem::path &cachePath, FileHandler::cache_t &t)
{
  std::unique_lock lck(mut_);
  cached_size_ += t->size();
  cached_files_.insert({cachePath.string(),t});
  if(cachePath.filename() == "index.html" || cachePath.filename() == "index.htm")
  {
    cached_files_.insert({cachePath.parent_path().string(),t});
  }
}

std::string FileHandler::__getFileMimetype(const std::filesystem::path &p)
{
  std::string file_type = "text/html";
  auto it = file_types_.find(p.extension());
  if(it!=file_types_.end())
    file_type = it->second;
  return file_type;
}


void FileHandler::AddMountPoint(std::filesystem::path pt)
{
  mount_points_.push_back(pt);
  std::thread t1([pt,this](){this->__iterateDirAndCacheFiles(pt);std::cout<<"Processed mount point "<<pt<<" total files indexed: "<<cached_files_.size()<<" total memory in use: "<<cached_size_<<" Bytes."<<std::endl;});
  t1.detach();
}
      
bool FileHandler::__loadFile(const std::filesystem::path &p, cache_t &ptr)
{
  std::ifstream ifs(p, std::ios::in);
  if(!ifs.is_open())
    return false;

  std::error_code ec;
  auto size = std::filesystem::file_size(p,ec);
  if(ec)
    return false;
  
  if(!ptr)
    ptr = std::shared_ptr<std::vector<char>>(new std::vector<char>());
  ptr->resize(size);

  ifs.read(ptr->data(),size);
  ifs.close();
  return true;
}

void FileHandler::ServeFile(const httplib::Request &req, httplib::Response &resp)
{
  
  {
    std::shared_lock lck(mut_);
    auto find_val = cached_files_.find(req.path);
    if(find_val!=cached_files_.end())
    {
      resp.status = 200;
      resp.set_content(const_cast<const char*>(find_val->second->data()),find_val->second->size(),__getFileMimetype(req.path).c_str());
      return;
    }
  }
  
  
  for(auto &it : mount_points_)
  {
    std::filesystem::path pt = it.string()+req.path;
    if(std::filesystem::is_directory(pt))
      pt=pt/"index.html";

    if(std::filesystem::exists(pt))
    {
      cache_t fl;
      if(!__loadFile(pt,fl))
        return;
      
      resp.status = 200;
      resp.set_content(const_cast<const char*>(fl->data()),fl->size(),__getFileMimetype(pt).c_str());

      if(cache_file_callback_(pt))
        __cacheFile(pt,fl);
    }
  }
}
