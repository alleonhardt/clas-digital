#ifndef CLASDIGITAL_SRC_SERVER_FILEHANDLER_H
#define CLASDIGITAL_SRC_SERVER_FILEHANDLER_H
#include <filesystem>
#include <atomic>
#include <unordered_map>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#endif

#include <shared_mutex>
#include <fstream>

namespace clas_digital
{
  template<typename T>
  bool atomic_write_file(std::filesystem::path p, const T &t1)
  {
    static std::atomic<int> x(0);
    auto tmpname = ".tmpfile"+std::to_string(x++);
    std::ofstream ofs(tmpname,std::ios::out);
    if(!ofs.is_open())
      return false;
    ofs<<t1;
    ofs.close();

    std::error_code ec;
    std::filesystem::rename(tmpname, p,ec);
    if(ec.value() != 0)
    {
      std::filesystem::remove(tmpname,ec);
      return false;
    }
    return true;
  }

  class FileHandler
  {
    public:
      FileHandler();
      void AddMountPoint(std::filesystem::path pt);
      void ServeFile(const httplib::Request &req, httplib::Response &resp);

    private:
      std::vector<std::filesystem::path> mount_points_;
      std::map<std::string,std::string> file_types_;
      std::unordered_map<std::string,std::vector<char>> cached_files_;
      std::shared_mutex mut_;
  };
}


#endif
