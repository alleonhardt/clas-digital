#ifndef CLASDIGITAL_SRC_SERVER_FILEHANDLER_UTIL_H
#define CLASDIGITAL_SRC_SERVER_FILEHANDLER_UTIL_H

#include <atomic>
#include <filesystem>
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
}
#endif
