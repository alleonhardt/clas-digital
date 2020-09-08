#ifndef CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H
#define CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H



#include <iostream>
#include <filesystem>
#include <functional>
#define WIN32_LEAN_AND_MEAN

#ifdef _WIN32

#ifdef CLASDIGITAL_PLUGIN
#define DLL_IMPORT _declspec(dllimport)
#else
#define DLL_IMPORT
#endif

#else
#define DLL_IMPORT
#endif
#include <termcolor/termcolor.hpp>

namespace debug
{
  /**
   * @brief Defines the log level when the global log level is lower than the
   * log level specified for the log, it wont be printed. This should prevent
   * cluttering of stdout
   */
  enum class LogLevel
  {
    ///< This gets always printed no matter how the global log level is
    DBG_ALWAYS = 1000,

    ///< This is used for errors when using global log level DBG_ERROR it will
    //print all errors and all logs with DBG_ALWAYS
    DBG_ERROR = 4,

    ///< This is used to print warnings, will get printed when the log level is
    //DBG_WARNING or higher
    DBG_WARNING = 3,

    ///< This if for debug messages and should ease error hunting
    DBG_DEBUG = 2,

    ///< This is for todos, when something should be fixed etc.
    DBG_TODO = 1
  };
  
  ///< The global log level when printing always log messages that have a higher
  //level than the global log level
  static inline LogLevel gLogLevel = LogLevel::DBG_ERROR;
 
  /**
   * @brief This log class should ease the user of colors and specific tags for
   * a group of messages and also to check if the message is even important with
   * the current log level
   */
  class LogClass
  {
    public:

      /**
       * @brief Creates the LogClass from the log level it should react to, a
       * string tag that should be printed at the begging and a color for this
       * particular log level.
       *
       * @param value The log level used to determine if the message is being
       * printed or now
       * @param tag The tag that is printed at the beginning of the message e.
       * g. "[ERROR] " for errors.
       * @param termcolor::red The color to use for this particular log message
       */
      LogClass(LogLevel value, const char *tag, decltype(termcolor::red) color) : level_(value), tag_(tag),color_(color)
      {
      }

      /**
       * @brief Check if the log level of this message is higher e. g. more
       * important than the current log level specified.
       *
       * @return 
       */
      bool IsActive()
      {
        return (int)level_ >= (int)gLogLevel;
      }

      
      /**
       * @brief Enables printing of the LogClass on std::cout, will change the
       * color to color_ and print the tag given in the constructor
       *
       * @param os The ostream to write to
       * @param lvl The LogClass instance about to be printed
       *
       * @return 
       */
      friend std::ostream& operator<<(std::ostream& os, const LogClass &lvl);

    private:
      ///< The log level of this message
      LogLevel level_;

      ///< The tag to print at the beginning of every message with this LogClass
      const char *tag_;

      ///< The color to use to print this message
      std::ostream& (*color_)(std::ostream&);
  };

  inline std::ostream& operator<<(std::ostream& os, const LogClass &lvl)
  {
    //First of all change the color and print the tag afterwards    
    return os<<lvl.color_<<lvl.tag_;
  }

  /**
   * @brief The LogClass for errors print all errors in red and with the "ERROR"
   * tag in front.
   */
  static inline LogClass LOG_ERROR{LogLevel::DBG_ERROR,"[ERROR] ",termcolor::red};


  /**
   * @brief The LogClass for warnings, print all warnings in yellow and with the
   * tag "[WARNING]" in front.
   */
  static inline LogClass LOG_WARNING{LogLevel::DBG_WARNING,"[WARNING] ",termcolor::yellow};


  /**
   * @brief The LogClass for debug messages, prints all debug messages in blue
   * and with the tag "[DEBUG]" in front.
   */
  static inline LogClass LOG_DEBUG{LogLevel::DBG_DEBUG,"[DEBUG] ",termcolor::blue};

  /**
   * @brief Logs a single value to stdout is the end of the variadic template
   * recursion of log_int
   *
   * @tparam T The type of the parameter about to get printed
   * @param t1 The parameter to get printed
   */
  template<typename T>
  void log_int(T t1)
  {
    std::cout<<t1;
  }


  /**
   * @brief Prints a variadic amount of arguments to stdout in the right order
   *
   * @tparam T The type of the current parameter to get printed to stdout
   * @tparam ...args The remaining arguments in the variadic function call
   * @param t1 The parameter about to get printed
   * @param arguments The remaining arguments
   */
  template<typename T, typename ...args>
  void log_int(T t1, args... arguments)
  {
    //Print the current first parameter and repeat with the remaining ones.
    std::cout<<t1;
    log_int(arguments...);
  }
 
  /**
   * @brief Handles the case, that the first parameter is a instance of LogClass
   *
   * @tparam ...args The remaining argument types.
   * @param lvl The LogClass determine if the logging will be continued
   * @param arguments The remaining arguments passed to the function
   */
  template<typename ...args>
  void log_int(LogClass lvl, args... arguments)
  {
    // Check if the log is even active, if not abort printing
    if(!lvl.IsActive())
      return;

    // Ok it is active change color print tag and return to printing the
    // remaining arguments
    std::cout<<lvl;
    log_int(arguments...);
  }

  /**
   * @brief Used to forward the received arguments to the variadic print
   * function and for resetting the color after logging.
   *
   * @tparam ...args
   * @param arguments
   */
  template<typename ...args>
  void log(args... arguments)
  {
    // Print all parameters then reset the color
    log_int(arguments...);
    std::cout<<termcolor::reset;
  }

  /**
   * @brief Registers a clean up function which will get executed in the
   * Destructor
   *
   * Simple example:
   * ```
   * {
   *  char *val = new char[256];
   *  CleanupDtor dt([val](){delete val;});
   *
   *  Dtor gets called!
   *  if(condition_x)
   *    throw 10;
   *  return 100;
   * }
   * ```
   */
  class CleanupDtor
  {
    public:

      /**
       * @brief Registers the cleanup function to use. Can be anything due to
       * lambda expressions in C++
       *
       * @param fnc The function to execute as soon as the lifetime of this
       * object ends
       */
      CleanupDtor(std::function<void()> &&fnc)
      {
        func_ = fnc;
      }


      /**
       * @brief The class is about to loose scope, so execute the cleanup
       * function
       */
      ~CleanupDtor()
      {
        func_();
      }
    private:
      ///< The function to execute as soon as the class gets destroyed
      std::function<void()> func_;
  };



  class PrintableError
  {
    public:
      virtual void print(int index=0,std::ostream &os=std::cout)
      {
        os<<color_;
        for(int i = 0; i < index; i++)
          os<<"==";
        if(index!=0)
          os<<"> ";
        os<<error_message_;
        os<<termcolor::reset;
      }
    protected:
      std::string error_message_;
      std::ostream& (*color_)(std::ostream&);
  };

  template<typename T>
  class Error : public PrintableError
  {
    public:
      Error(T err, std::string error_message = "",decltype(termcolor::red) color=termcolor::red) : error_code_(err) 
    {
      color_ = color;
      if(error_message == "")
      {
        if((int)err == 0)
        {
          error_message_ = "Error code 0. No error occured.\n";
          color_ = termcolor::reset;
        }
        else
          error_message_ = "Error Code: "+std::to_string((int)err)+".\n";
      }
      else
      {
        error_message_ = "Error Code "+std::to_string((int)err)+": \""+error_message+"\"\n";
      }
    }

      void print(int index = 0, std::ostream &os=std::cout) override
      {
        PrintableError::print(index,os);
        if(next_)
          next_->print(index+1,os);
      }

      operator bool() const
      {
        return (int)error_code_ != 0;
      }


      template<typename Y>
      Error<T> &Next(Error<Y> &ptr)
      {
        next_.reset(new Error<Y>(std::move(ptr)));
        return *this;
      }

      T GetErrorCode()
      {
        return error_code_;
      }

      void SetErrorCode(T new_val)
      {
        error_code_ = new_val;
      }

    protected:
      T error_code_;
      std::shared_ptr<PrintableError> next_;
  };


}

#define DBG_FULL_LOG "[",__FILE__,":",__LINE__,"] "
#define DBG_EXT_LOG "(",std::filesystem::path(__FILE__).filename().string().c_str(),":",__LINE__,") "


#endif
