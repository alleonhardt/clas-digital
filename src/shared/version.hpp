#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace version
{
  inline const char gGitSha[] = "3c3c6a82a4f20b950dc60f640c39e9f8e1f125a6-dirty";
  inline const char gDate[] = "Tue Sep 8 08:34:17 2020";
  inline const char gCommitSubject[] = "Still trying to fix errors on windows and apple";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
