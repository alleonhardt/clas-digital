#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace version
{
  inline const char gGitSha[] = "b012a8234c6ce3adad9b776aabbdcc02e4dc103b-dirty";
  inline const char gDate[] = "Mon Sep 7 12:55:39 2020";
  inline const char gCommitSubject[] = "Changed the execution directory of the continous integration tests to match the development directory";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
