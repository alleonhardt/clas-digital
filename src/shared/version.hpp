#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "14a4645cfe703012c04da5db9dcc2d0de3dc1f88-dirty";
  inline const char gDate[] = "Tue Feb 2 19:45:53 2021";
  inline const char gCommitSubject[] = "Merge branch 'cpp' of github.com:ShadowItaly/clas-digital into cpp";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
