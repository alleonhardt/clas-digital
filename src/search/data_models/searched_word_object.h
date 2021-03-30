#ifndef SRC_SEARCH_DATA_MODELS_SEARCHEDWORDOBJECT_H_
#define SRC_SEARCH_DATA_MODELS_SEARCHEDWORDOBJECT_H_

#include <string>

struct SearchedWordObject {
  std::string orginal_;
  std::string converted_;
  short scope_;
};

#endif
