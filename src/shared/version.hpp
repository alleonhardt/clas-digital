#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "317f07466e173a399953a4e5daaadd493568dafd";
  inline const char gDate[] = "Sat Nov 14 18:53:54 2020";
  inline const char gCommitSubject[] = "Fixed small compile mistake on apple and fixed the pathing in windows";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
