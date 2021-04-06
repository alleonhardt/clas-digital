
#include "result_object.h"
#include "func.hpp"
#include "searched_word_object.h"
#include <numeric>
#include <resolv.h>
#include <vector>

bool ResultObject::found_in_metadata() const {
  return scope()>1;
}

bool ResultObject::found_in_corpus() const {
  return 1&scope();
}

short ResultObject::scope() const {
  return std::accumulate(matches_.begin(), matches_.end(), 0, 
      [](const short& init, const auto& elem) { return init|elem.second.scope(); });
}

double ResultObject::score() const {
  return score_;
}

Book* ResultObject::book() const {
  return book_;
}

std::map<std::string, FoundWordsObject> ResultObject::matches() const {
  return matches_;
}

std::vector<FoundWordsObject> ResultObject::matches_as_list() const {
  std::vector<FoundWordsObject> matches;
  for (const auto& it : matches_) 
    matches.push_back(it.second);
  return matches;
}

void ResultObject::set_score(double score) {
  score_ = score;
}

void ResultObject::set_book(Book* book) {
  book_ = book; 
}

void ResultObject::set_original_words(
    std::map<std::string, std::string> converted_to_original_map) {
  for (auto it : converted_to_original_map) {
    if (matches_.count(it.first) > 0)
      matches_[it.first].original_word_ = it.second;
  }
}

void ResultObject::NewResult(std::string searched_word, std::string found_word, 
    short scope, double score) {
  // Update found matches. 
  matches_[searched_word].AddWord(found_word, scope, score);
  
  // Increase score.
  score_ += score;
}

void ResultObject::Join(ResultObject &result_object) {
  // Add score.
  score_ += result_object.score();
  
  // (Bitwise) add scopes for each word.
  for (const auto& it : result_object.matches()) 
    matches_[it.first] = it.second;
}


void ResultObject::Print(std::string word, std::string preview) {
  if (!found_in_corpus()) return;
  std::cout << "--- " << book_->key() << " --- " << std::endl;  
  std::cout << "Scope: " << scope_ << std::endl;
  std::cout << "Score: " << score_ << std::endl;

  std::cout << "Found the following matches: " << std::endl;
  for (const auto& word : matches_) {
    for (const auto& it : word.second.matched_words_) 
      std::cout << it.first << std::endl;
  }
  std::cout << "Preview: " << preview << std::endl;
  std::cout << std::endl;
}
