#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "37fc76d56ff8cd6aae21a88c952e175ed46454db-dirty";
  inline const char gDate[] = "Thu Sep 10 20:36:07 2020";
  inline const char gCommitSubject[] = "Merge branch 'cpp' of github.com:ShadowItaly/clas-digital into cpp";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
