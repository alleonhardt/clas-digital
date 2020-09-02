#ifndef CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H
#define CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H



#include <iostream>
#include <cstdio>
#include <termcolor/termcolor.hpp>

namespace debug
{
	static inline volatile bool gGlobalShutdown = false;
 

  enum class LOG_LEVEL 
  {
    ALL = 0,
    DEBUG = 1,
    WARNING = 2,
    ERRORS = 4,
    NONE = 8
  };

  static inline LOG_LEVEL gLogLevel = LOG_LEVEL::ALL;

  enum class ExitType
  {
    OK = 0,
    FAILURE = -1
  };

  template<typename ...variadic>
  void print_exit(ExitType tp,variadic ...args)
  {
    printf(args...);
    std::exit((int)tp);
  }

  template<typename ...variadic>
  void print_exit(std::ostream& (&val)(std::ostream&), ExitType tp, variadic ...args)
  {
    val(std::cout).flush();
    print_exit(tp, args...);
    termcolor::reset(std::cout).flush();
  }

  template<typename ...args>
  void print(args ...arg)
  {
    printf(arg...);
  }

  template<typename ...args>
  void print(std::ostream& (&val)(std::ostream&),args ...arg)
  {
    val(std::cout).flush();
    print(arg...);
    termcolor::reset(std::cout).flush();
  }

  template<typename ...args>
  void log(args ...arg)
  {
    printf(arg...);
  }
  
  template<>
  inline void print<const char*>(const char *arg)
  {
    puts(arg);
  }

  template<>
  inline void log<const char*>(const char *arg)
  {
    puts(arg);
  }
  
  template<typename ...args>
  void log(LOG_LEVEL lvl,args ...arg)
  {
    if((int)lvl >= (int)gLogLevel)
    {
      if(lvl == LOG_LEVEL::ERRORS)
      {
        termcolor::red(std::cout).flush();
      }
      else if(lvl == LOG_LEVEL::WARNING)
      {
        termcolor::yellow(std::cout).flush();
      }
      else if(lvl == LOG_LEVEL::DEBUG)
      {
        termcolor::magenta(std::cout).flush();
      }
      print(arg...);
      termcolor::reset(std::cout).flush();
    }
  }
}



#endif
