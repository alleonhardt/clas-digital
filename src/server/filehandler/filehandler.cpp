#include "filehandler.hpp"

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
}

void IndexFiles(std::filesystem::path pt)
{
  int indexed_files = 0;
  unsigned long long size = 0;

  for(auto &it : std::filesystem::recursive_directory_iterator(pt))
  {
    if(!it.is_directory() && (it.path().extension() == ".html" || it.path().extension() == ".css" || it.path().extension() == ".js" || it.path().extension() == ".json"))
    {
      indexed_files++;
      if((indexed_files%1000) == 0)
        std::cout<<"Indexed "<<indexed_files<<std::endl;
      size+=it.file_size();
    }
  }
  std::cout<<"Found "<<indexed_files<<" files with a total size of "<<(float)size/(1024*1024)<<" MB"<<std::endl;
}


void FileHandler::AddMountPoint(std::filesystem::path pt)
{
  mount_points_.push_back(pt);
  std::cout<<"Add mount point: "<<pt<<std::endl;
  IndexFiles(pt);
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
      std::string file_type = "text/html";
      auto it = file_types_.find(std::filesystem::path(req.path).extension());
      if(it!=file_types_.end())
        file_type = it->second;


      resp.set_content(find_val->second.data(),find_val->second.size(),file_type.c_str());
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
      std::string file_type = "text/html";
      auto it = file_types_.find(pt.extension());
      if(it!=file_types_.end())
        file_type = it->second;

      
      resp.status = 200;
      resp.set_content(vec.data(),vec.size(),file_type.c_str());

      std::unique_lock lck(mut_);
      cached_files_.insert({req.path,std::move(vec)});
    }
  }
}
