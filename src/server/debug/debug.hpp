#ifndef CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H
#define CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H



#include <iostream>
#include <filesystem>
#include <termcolor/termcolor.hpp>

namespace debug
{
	static inline volatile bool gGlobalShutdown = false;
  

  static inline int gLogLevel = 0;
 
  class LogLevel
  {
    public:
      LogLevel(int value, const char *tag, decltype(termcolor::red) color) : level_(value), tag_(tag),color_(color)
      {
      }

      bool IsActive()
      {
        return level_ >= gLogLevel;
      }
      friend std::ostream& operator<<(std::ostream& os, const LogLevel &lvl);

    private:
      int level_;
      const char *tag_;
      std::ostream& (*color_)(std::ostream&);
  };

  inline std::ostream& operator<<(std::ostream& os, const LogLevel &lvl)
  {
    
    return os<<lvl.color_<<lvl.tag_;
  }

  static inline LogLevel LOG_ERROR{16,"[ERROR] ",termcolor::red};
  static inline LogLevel LOG_WARNING{32,"[YELLOW] ",termcolor::yellow};
  static inline LogLevel LOG_DEBUG{64,"[DEBUG] ",termcolor::blue};

  template<typename T>
  void log_int(T t1)
  {
    std::cout<<t1;
  }

  template<typename T, typename ...args>
  void log_int(T t1, args... arguments)
  {
    std::cout<<t1;
    log_int(arguments...);
  }
  
  template<typename ...args>
  void log(args... arguments)
  {
    log_int(arguments...);
    std::cout<<termcolor::reset;
  }
}
#define DBG_FULL_LOG "[",__FILE__,":",__LINE__,"] "
#define DBG_EXT_LOG "[",std::filesystem::path(__FILE__).filename().string().c_str(),":",__LINE__,"] "



#endif
