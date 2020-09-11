#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "2148bf3ace28cd095f2f475444a28c585176e72a-dirty";
  inline const char gDate[] = "Fri Sep 11 17:28:21 2020";
  inline const char gCommitSubject[] = "Fixed bug when presenting search results";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
