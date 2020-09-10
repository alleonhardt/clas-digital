#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "1d4c70ab2b82c5e6da6100512952441ab255d8d7-dirty";
  inline const char gDate[] = "Wed Sep 9 12:55:44 2020";
  inline const char gCommitSubject[] = "Finished the update of the usertable to be more extensible";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
