
#include "search_object.h"
#include "func.hpp"
#include "search_options.h"
#include <algorithm>

SearchObject::SearchObject(std::string query, SearchOptions search_options) 
    : query_(query), search_options_(search_options) {
 
  // replace all spaces by "+" as the user may use " " or "+" as a delimitter.
  std::string replaced_spaces = query;
  std::replace(replaced_spaces.begin(), replaced_spaces.end(), ' ', '+');

  // Get all words and remove all empty words.
  words_ = func::split2(replaced_spaces, "+");
  words_.erase(std::remove_if(words_.begin(), words_.end(), 
        [](std::string x) { return x == ""; }), words_.end());

  // Convert all words to lower and replace non utf-8 characters.
  for (auto word : words_) {
    std::cout << word << std::endl;
    // TODO (fux): reduce to base-form.
    converted_words_.push_back(func::convertStr(func::returnToLower(word)));
  }
}

std::string SearchObject::query() const {
  return query_;
}
const std::vector<std::string>& SearchObject::words() const {
  return words_;
}
const std::vector<std::string>& SearchObject::converted_words() const {
  return converted_words_;
}
SearchOptions& SearchObject::search_options() {
  return search_options_;
}
