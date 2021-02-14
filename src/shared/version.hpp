#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "6490fda1a5d5e3ad19b99109c3b0f26db4856ea6-dirty";
  inline const char gDate[] = "Fri Feb 12 18:45:03 2021";
  inline const char gCommitSubject[] = "Merge branch 'new_search' of github.com:ShadowItaly/clas-digital into new_search";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
