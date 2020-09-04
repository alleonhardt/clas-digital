#ifndef CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H
#define CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H



#include <iostream>
#include <filesystem>
#include <termcolor/termcolor.hpp>

namespace debug
{
  enum class LogLevel
  {
    ALWAYS = 0,
    ERROR = 1,
    WARNING = 2,
    DEBUG = 3,
    NONE = 4
  };
  
  static inline LogLevel gLogLevel = LogLevel::ALWAYS;
 
  class LogClass
  {
    public:
      LogClass(LogLevel value, const char *tag, decltype(termcolor::red) color) : level_(value), tag_(tag),color_(color)
      {
      }

      bool IsActive()
      {
        return (int)level_ >= (int)gLogLevel;
      }
      friend std::ostream& operator<<(std::ostream& os, const LogClass &lvl);

    private:
      LogLevel level_;
      const char *tag_;
      std::ostream& (*color_)(std::ostream&);
  };

  inline std::ostream& operator<<(std::ostream& os, const LogClass &lvl)
  {
    
    return os<<lvl.color_<<lvl.tag_;
  }

  static inline LogClass LOG_ERROR{LogLevel::ERROR,"[ERROR] ",termcolor::red};
  static inline LogClass LOG_WARNING{LogLevel::WARNING,"[WARNING] ",termcolor::yellow};
  static inline LogClass LOG_DEBUG{LogLevel::DEBUG,"[DEBUG] ",termcolor::blue};

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
#define DBG_EXT_LOG "(",std::filesystem::path(__FILE__).filename().string().c_str(),":",__LINE__,") "



#endif
