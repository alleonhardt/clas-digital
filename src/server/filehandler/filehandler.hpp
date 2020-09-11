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
  
  class FileHandler
  {
    public:
      using cache_t = std::shared_ptr<std::vector<char>>;

      FileHandler();
      void AddMountPoint(std::filesystem::path pt);
      void ServeFile(const httplib::Request &req, httplib::Response &resp);

    private:
      std::vector<std::filesystem::path> mount_points_;
      std::map<std::string,std::string> file_types_;

      std::function<bool(const std::filesystem::path&)> cache_file_callback_;

      std::unordered_map<std::string,cache_t> cached_files_;
      unsigned long long cached_size_;
      std::shared_mutex mut_;

      bool __loadFile(const std::filesystem::path &p, cache_t &ptr);
      std::string __getFileMimetype(const std::filesystem::path &mime);
      void __cacheFile(const std::filesystem::path &cachePath);
      void __cacheFile(const std::filesystem::path &cachePath, cache_t &t);
      void __iterateDirAndCacheFiles(const std::filesystem::path &p);
  };
}


#endif
