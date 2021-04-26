#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "327d03a764321cb857886dc7966c1ded523f00a4-dirty";
  inline const char gDate[] = "Mon Apr 26 16:52:05 2021";
  inline const char gCommitSubject[] = "Merge branch 'cpp' of github.com:ShadowItaly/clas-digital into cpp";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
