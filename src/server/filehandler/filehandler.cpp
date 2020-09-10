#include "filehandler.hpp"

using namespace clas_digital;

void FileHandler::AddMountPoint(std::filesystem::path pt)
{
  mount_points_.push_back(pt);
}

void FileHandler::ServeFile(const httplib::Request &req, httplib::Response &resp)
{
  
  decltype(cached_files_.find(req.path)) find_val;
  {
    std::shared_lock lck(mut_);
    find_val = cached_files_.find(req.path);
  }
  if(find_val!=cached_files_.end())
  {
      resp.status = 200;
      resp.set_content(find_val->second.data(),find_val->second.size(),"text/html");
      return;
  }
  
  
  for(auto &it : mount_points_)
  {
    std::filesystem::path pt = it.string()+req.path;
    if(std::filesystem::is_directory(pt))
      pt=pt/"index.html";

    if(std::filesystem::exists(pt))
    {
      std::ifstream ifs(pt, std::ios::in);
      if(!ifs.is_open())
        return;

      std::error_code ec;
      
      std::vector<char> vec;
      auto size = std::filesystem::file_size(pt,ec);
      vec.resize(size);
      if(ec)
        return;

      ifs.read(vec.data(),size);
      ifs.close();
      
      resp.status = 200;
      resp.set_content(vec.data(),vec.size(),"text/html");

      std::unique_lock lck(mut_);
      cached_files_.insert({req.path,std::move(vec)});
    }
  }
}
