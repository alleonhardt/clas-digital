#ifndef SRC_SEARCH_DATA_MODELS_SEARCHEDWORDOBJECT_H_
#define SRC_SEARCH_DATA_MODELS_SEARCHEDWORDOBJECT_H_

#include <iterator>
#include <list>
#include <numeric>
#include <string>

struct FoundWordsObject {
  std::string original_word_;
  double best_score_;
  std::list<std::pair<std::string, short>> matched_words_;

  short scope() const {
    return std::accumulate(matched_words_.begin(), matched_words_.end(), 0,
        [](const short& init, const auto& elem) { return init|elem.second; });
  }

  void AddWord(std::string match, short scope, double score) {
    if (best_score_ >= score) 
      matched_words_.push_back({match, scope});
    else {
      matched_words_.push_front({match, scope});
      best_score_ = score; 
    }
  }
};

#endif
