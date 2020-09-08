#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace version
{
  inline const char gGitSha[] = "d043c34fdf89f58272fd2f8c8e37078335e420ee-dirty";
  inline const char gDate[] = "Tue Sep 8 11:42:09 2020";
  inline const char gCommitSubject[] = "Improved library detection for mac and improved compilation on windows";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
