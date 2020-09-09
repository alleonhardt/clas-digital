#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "42ba53395884431f622919215e296c0abc603c2f-dirty";
  inline const char gDate[] = "Tue Sep 8 20:13:38 2020";
  inline const char gCommitSubject[] = "Merge branch 'cpp' of https://github.com/ShadowItaly/clas-digital into cpp";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
