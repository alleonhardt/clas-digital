#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace version
{
  inline const char gGitSha[] = "d8663db9b8eed3bbe04d29251386cbe2f648427a-dirty";
  inline const char gDate[] = "Tue Sep 8 10:54:50 2020";
  inline const char gCommitSubject[] = "Trying another time to fix errors on windows and mac";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
