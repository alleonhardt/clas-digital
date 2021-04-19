#include "search_options.h"
#include "func.hpp"

SearchOptions::SearchOptions(bool fuzzy_search, bool only_metadata, bool only_corpus,
    int year_from, int year_to, int sort_results_by, std::string author,
    std::vector<std::string> collections) : 
      fuzzy_search_(fuzzy_search),
      only_metadata_(only_metadata),
      only_corpus_(only_corpus),
      year_from_(year_from),
      year_to_(year_to),
      sort_result_by_(sort_results_by),
      author_(func::ReplaceMultiByteChars(func::ReturnToLower(author))),
      collections_(collections) {}

SearchOptions::SearchOptions(bool fuzzy_search) :
      fuzzy_search_(fuzzy_search),
      only_metadata_(false),
      only_corpus_(false),
      year_from_(0),
      year_to_(2050),
      sort_result_by_(0),
      author_(""),
      collections_({}) {}

bool SearchOptions::fuzzy_search() const {
  return fuzzy_search_;
}
bool SearchOptions::only_metadata() const {
  return only_metadata_;
}
bool SearchOptions::only_corpus() const {
  return only_corpus_;
}
int SearchOptions::year_from() const {
  return year_from_;
}
int SearchOptions::year_to() const {
  return year_to_;
}
int SearchOptions::sort_results_by() const {
  return sort_result_by_;
}
std::string SearchOptions::author() const {
  return author_;
}
std::vector<std::string> SearchOptions::collections() const {
  return collections_;
}

