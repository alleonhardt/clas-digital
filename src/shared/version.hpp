#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "dc2352345027a1be4ea68ce82a939b83c99af956-dirty";
  inline const char gDate[] = "Wed Feb 3 22:36:14 2021";
  inline const char gCommitSubject[] = "Added the functionality for standalone ocr and standalone img";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
