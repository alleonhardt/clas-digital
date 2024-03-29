#ifndef CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H
#define CLASIDIGITAL_SRC_SERVER_DEBUG_DEBUG_H

#include <iostream>
#include <filesystem>
#include <functional>
#include <type_traits>
#define WIN32_LEAN_AND_MEAN

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
 
  /**
   * @brief This log class should ease the user of colors and specific tags for
   * a group of messages and also to check if the message is even important with
   * the current log level
   */
  class LogClass
  {
    public:
      static LogLevel gLogLevel;

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
        dtor_active_ = true;
        func_ = fnc;
      }

      CleanupDtor()
      {
        dtor_active_ = false;
      }

      void SetFunction(std::function<void()> &&fnc)
      {
        if(dtor_active_)
          func_();
        dtor_active_ = true;
        func_ = std::move(fnc);
      }


      /**
       * @brief The class is about to loose scope, so execute the cleanup
       * function
       */
      ~CleanupDtor()
      {
        if(dtor_active_)
          func_();
      }
    private:
      ///< The function to execute as soon as the class gets destroyed
      std::function<void()> func_;
      bool dtor_active_;
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

namespace clas_digital
{
  struct trace_info
  {
    const char *file;
    const char *function;
    int line;
  };



  template<typename ReturnType,typename ErrorType, bool SaveTraceInfo=true>
  class clas_error
  {
    public:
      clas_error(ReturnType ret)
      {
        has_error_ = false;
        ret_ = ret;
      }

      clas_error(ErrorType t)
      {
        static_assert(SaveTraceInfo,"Specified save trace information, but did not provide tracing information!");
      }

      clas_error(ErrorType t, trace_info inf)
      {
        has_error_ = true;
        error_ = t;
        if constexpr(SaveTraceInfo)
          trace_info_ = inf;
      }

      template<typename NewReturnType>
      clas_error<ErrorType,NewReturnType> &map(std::function<clas_error<ErrorType,NewReturnType>&(ReturnType)> fnc)
      {
        if(has_error_)
        {
          has_error_ = false;
          return clas_error<ErrorType,NewReturnType>(error_);
        }
        return fnc(ret_);
      }

      template<typename NewReturnType>
      clas_error<ErrorType,NewReturnType> &map(std::function<clas_error<ErrorType,NewReturnType>&()> fnc)
      {
        if(has_error_)
        {
          has_error_ = false;
          return clas_error<ErrorType,NewReturnType>(error_);
        }
        return fnc();
      }

      void error(std::function<void(ErrorType)> fnc)
      {
        if(has_error_)
        {
          has_error_ = false;
          fnc(error_);
        }
      }

      clas_error<ErrorType,ReturnType> &print()
      {
        if(has_error_)
        {
          std::cout<<" -> Contains error\n";
          std::cout<<" -> -> Error: "<<error_<<std::endl;
          if constexpr(SaveTraceInfo)
          {
            std::cout<<" -> -> Function: "<<trace_info_.function<<std::endl;
            std::cout<<" -> -> File: "<<trace_info_.file<<std::endl;
            std::cout<<" -> -> Line: "<<trace_info_.line<<std::endl;
          }
        }
        else
        {
          std::cout<<" -> Contains value"<<std::endl;
          std::cout<<" -> -> Value: "<<ret_<<std::endl;
        }
        return *this;
      }

      ~clas_error()
      {
        if(has_error_)
        {
          std::cout<<termcolor::red<<"Unchecked error, terminating programm. Error details: \n"<<std::endl;
          this->print();
          std::cout<<termcolor::reset;
          throw *this;
        }
      }

    protected:
      ErrorType error_;
      ReturnType ret_;
      trace_info trace_info_;
      bool has_error_;
  };
}

#ifdef _WIN32
#define CL_TRACE clas_digital::trace_info{.file=__FILE__,.function=__FUNC_SIG__,.line=__LINE__}
#else
#define CL_TRACE clas_digital::trace_info{.file=__FILE__,.function=__PRETTY_FUNCTION__,.line=__LINE__}
#endif

#define GET_MACRO(_1,_2,NAME,...) NAME
#define cl_create_err(...) GET_MACRO(__VA_ARGS__, cl_create_err2, cl_create_err1)(__VA_ARGS__)


#define DBG_FULL_LOG "[",__FILE__,":",__LINE__,"] "
#define DBG_EXT_LOG "(",std::filesystem::path(__FILE__).filename().string().c_str(),":",__LINE__,") "



#endif
