#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace version
{
  inline const char gGitSha[] = "e38cfe32bad3d19cf07b47ace60610cd37803471-dirty";
  inline const char gDate[] = "Tue Sep 8 11:11:18 2020";
  inline const char gCommitSubject[] = "Trying another fix";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
