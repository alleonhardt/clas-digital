#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "e1626aa076bad2b39a9edcd25dca00f60f08eded-dirty";
  inline const char gDate[] = "Thu Sep 17 03:18:38 2020";
  inline const char gCommitSubject[] = "Fixed merge conflict and added a bit more caching infrastructure";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
