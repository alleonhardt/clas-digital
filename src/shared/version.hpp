#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace version
{
  inline const char gGitSha[] = "628e465ab22714c7764b7572758492c3d0a7cbf1-dirty";
  inline const char gDate[] = "Mon Sep 7 22:08:04 2020";
  inline const char gCommitSubject[] = "Merge branch 'cpp' of github.com:ShadowItaly/clas-digital into cpp";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
