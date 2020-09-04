#ifndef CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H
#define CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H



#include <iostream>
#include <filesystem>
#include <functional>
#include <termcolor/termcolor.hpp>

namespace debug
{
  enum class LogLevel
  {
    DBG_ALWAYS = 0,
    DBG_ERROR = 1,
    DBG_WARNING = 2,
    DBG_DEBUG = 3,
    DBG_TODO = 4
  };
  
  static inline LogLevel gLogLevel = LogLevel::DBG_DEBUG;
 
  class LogClass
  {
    public:
      LogClass(LogLevel value, const char *tag, decltype(termcolor::red) color) : level_(value), tag_(tag),color_(color)
      {
      }

      bool IsActive()
      {
        return (int)level_ <= (int)gLogLevel;
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

  static inline LogClass LOG_ERROR{LogLevel::DBG_ERROR,"[ERROR] ",termcolor::red};
  static inline LogClass LOG_WARNING{LogLevel::DBG_WARNING,"[WARNING] ",termcolor::yellow};
  static inline LogClass LOG_DEBUG{LogLevel::DBG_DEBUG,"[DEBUG] ",termcolor::blue};

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
  void log_int(LogClass lvl, args... arguments)
  {
    if(!lvl.IsActive())
      return;
    std::cout<<lvl;
    log_int(arguments...);
  }

  template<typename ...args>
  void log(args... arguments)
  {
    log_int(arguments...);
    std::cout<<termcolor::reset;
  }

  class CleanupDtor
  {
    public:
      CleanupDtor(std::function<void()> &&fnc)
      {
        func_ = fnc;
      }

      ~CleanupDtor()
      {
        func_();
      }
    private:
      std::function<void()> func_;
  };
}

#define DBG_FULL_LOG "[",__FILE__,":",__LINE__,"] "
#define DBG_EXT_LOG "(",std::filesystem::path(__FILE__).filename().string().c_str(),":",__LINE__,") "



#endif
