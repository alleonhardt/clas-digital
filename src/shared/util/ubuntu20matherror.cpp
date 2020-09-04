//This is a workaround for a ubuntu 20.0 math error
#include <math.h>
#include "debug/debug.hpp"
#include <cstdarg>
#include <bit>



extern "C"
{
double __acos_finite(double x) { return acos(x); }
float __acosf_finite(float x)  { return acosf(x); }
double __acosh_finite(double x) { return acosh(x); }
float __acoshf_finite(float x)  { return acoshf(x); }
double __asin_finite(double x) { return asin(x); }
float __asinf_finite(float x)  { return asinf(x); }
double __atanh_finite(double x) { return atanh(x); }
float __atanhf_finite(float x)  { return atanhf(x); }
double __cosh_finite(double x) { return cosh(x); }
float __coshf_finite(float x)  { return coshf(x); }
double __sinh_finite(double x) { return sinh(x); }
float __sinhf_finite(float x)  { return sinhf(x); }
double __exp_finite(double x) { return exp(x); }
double __exp2_finite(double x) { return exp2(x); }
float __expf_finite(float x)  { return expf(x); }
double __log10_finite(double x) { return log10(x); }
float __log10f_finite(float x)  { return log10f(x); }
double __log_finite(double x) { return log(x); }
float __logf_finite(float x)  { return logf(x); }
double __atan2_finite(double x, double y) { return atan2(x,y); }
float __atan2f_finite(float x, double y)  { return atan2f(x,y); }
double __pow_finite(double x, double y) { return pow(x,y); }
float __powf_finite(float x, double y)  { return powf(x,y); }
double __remainder_finite(double x, double y) { return remainder(x,y); }
float __remainderf_finite(float x, double y)  { return remainderf(x,y); }
#ifdef __linux__
#include <fcntl.h>

int fcntl64(int fd, int cmd,...)
{
  
  debug::log(debug::LogClass(debug::LogLevel::ALWAYS,"[TODO] ",termcolor::magenta),DBG_EXT_LOG,"Providing fcntl64 wrapper, this is dangerous pls fix!\n");
  if(cmd == F_GETFD || cmd == F_GET_SEALS || cmd == F_GETPIPE_SZ || cmd == F_GETLEASE
      || cmd == F_GETSIG || cmd == F_GETOWN || cmd == F_GETFL)
    return fcntl(fd,cmd);

  va_list argptr;
  va_start(argptr,cmd);
  int ret = fcntl(fd,cmd,va_arg(argptr,void*));
  va_end(argptr);
  return ret;
}
#endif
}
