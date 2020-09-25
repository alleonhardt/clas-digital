#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "30fcaeacf36c0975a015c7a3de9aaa6af8627e74-dirty";
  inline const char gDate[] = "Sat Sep 19 00:32:02 2020";
  inline const char gCommitSubject[] = "Fixed a few bugs in the new zotero update cache function and added a thread to dispatch events when updating 100+ new references.";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
