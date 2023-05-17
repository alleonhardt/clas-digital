#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "ce6e19d7cb65e650f83f36321a0966cea583d55d-dirty";
  inline const char gDate[] = "Wed Apr 6 12:55:55 2022";
  inline const char gCommitSubject[] = "Adds instructions in README in case of glibc link error";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
