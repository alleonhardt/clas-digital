
#include "result_object.h"
#include "func.hpp"
#include "searched_word_object.h"
#include <resolv.h>
#include <vector>

bool ResultObject::found_in_metadata() const {
  return scope_>1;
}
bool ResultObject::found_in_corpus() const {
  return 1&scope_;
}
short ResultObject::scope() const {
  return scope_;
}
double ResultObject::score() const {
  return score_;
}
Book* ResultObject::book() const {
  return book_;
}
std::map<std::string, short> ResultObject::words_with_scope() const {
  return words_with_scope_;
}
void ResultObject::set_book(Book* book) {
  book_ = book; 
}

void ResultObject::NewResult(std::string word, short scope, double score) {
  // Update all-over scope. 
  scope_ |= scope; 

  // Update scope for each word.
  words_with_scope_[word] |= scope;

  // Increase score.
  score_ += score;
}

void ResultObject::Join(ResultObject &result_object) {
  // Update scope.
  scope_ |= result_object.scope();

  // Add score.
  score_ += result_object.score();
  
  // (Bitwise) add scopes for each word.
  for (const auto& it : result_object.words_with_scope()) 
    words_with_scope_[it.first] |= it.second;
}


std::vector<SearchedWordObject> ResultObject::GetSearchWords(
    std::map<std::string, std::string> converted_to_original_map) {
  std::vector<SearchedWordObject> searched_words;
  for (auto it : converted_to_original_map) {
    if (words_with_scope_.count(it.first) > 0) {
      searched_words.push_back({it.second, it.first, words_with_scope_[it.first]});
    }
  }
  return searched_words;
}


void ResultObject::Print(std::string word, std::string preview) {
  if (!found_in_corpus()) return;
  std::cout << "--- " << book_->key() << " --- " << std::endl;  
  std::cout << "Found metadata: " << found_in_metadata() << std::endl;
  std::cout << "Found corpus: " << found_in_corpus() << std::endl;
  std::cout << "Scope: " << scope_ << std::endl;
  std::cout << "Score: " << score_ << std::endl;

  for (const auto& word : words_with_scope_) {
    std::cout << word.first << ": " << word.second << std::endl;
  
    if (book_->words_on_pages().count(word.first) > 0)
      std::cout << "Direct match (corpus): true" << std::endl;
    else if (book_->words_in_tags().count(word.first) > 0)
      std::cout << "Direct match (metadata): true" << std::endl;
    else if (book_->corpus_fuzzy_matches().count(word.first) > 0)
      std::cout << "Fuzzy match (corpus): " << book_->corpus_fuzzy_matches()[word.first].GetBestMatch() << std::endl;
    else if (book_->metadata_fuzzy_matches().count(word.first) > 0)
      std::cout << "Fuzzy match (metadata): " << book_->metadata_fuzzy_matches()[word.first].GetBestMatch() << std::endl;
    else 
      std::cout << "Something went wrong: no match could be tracked. (Or direct match in title/ author)" << std::endl;
  }
  std::cout << "Preview: " << preview << std::endl;
  std::cout << std::endl;
}
