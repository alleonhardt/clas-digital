#ifndef SRC_SEARCH_DATA_MODELS_MATCHOBJECT_H_
#define SRC_SEARCH_DATA_MODELS_MATCHOBJECT_H_

#include <iostream>

struct MatchObject {
  double relevance_;
  short scope_; // Each set bit represents a differnt scope.
};

#endif
