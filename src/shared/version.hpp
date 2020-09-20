#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "3228acd504249abcabc7a081af8ec86a20b15f2a";
  inline const char gDate[] = "Sun Sep 13 20:57:19 2020";
  inline const char gCommitSubject[] = "Added gramma-class to generate better results concerning relevant neighbours";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
