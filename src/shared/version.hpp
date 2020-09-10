#ifndef CLASDIGITAL_SRC_SHARED_VERSION_HPP
#define CLASDIGITAL_SRC_SHARED_VERSION_HPP
#include <string>
#include <termcolor/termcolor.hpp>

namespace clas_digital
{
  inline const char gGitSha[] = "d26e59b03d98715be5443092cbf4274afe58ee41-dirty";
  inline const char gDate[] = "Thu Sep 10 17:11:42 2020";
  inline const char gCommitSubject[] = "Changed request for search in book to match new request name >>api/v2/search/pages<< to direkt to search-engine";

  inline void print_version()
  {
    std::cout<<termcolor::cyan<<"Compiled at "<<gDate<<" with commit SHA1: "<<gGitSha<<" and commit subject: "<<gCommitSubject<<termcolor::reset<<std::endl;
  }
};


#endif
