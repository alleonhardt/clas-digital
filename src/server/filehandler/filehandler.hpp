#ifndef CLASDIGITAL_SRC_SERVER_FILEHANDLER_H
#define CLASDIGITAL_SRC_SERVER_FILEHANDLER_H
#include <filesystem>
#include <atomic>
#include <unordered_map>
#include <httplib.h>
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
    ofs<<t1;
    ofs.close();

    std::error_code ec;
    std::filesystem::rename(tmpname, p,ec);
    return ec.value() == 0;
  }

  class FileHandler
  {
    public:
      void AddMountPoint(std::filesystem::path pt);
      void ServeFile(const httplib::Request &req, httplib::Response &resp);

    private:
      std::vector<std::filesystem::path> mount_points_;
      std::unordered_map<std::string,std::vector<char>> cached_files_;
      std::shared_mutex mut_;
  };
}


#endif
