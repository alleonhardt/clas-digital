#include "search.h"
#include "func.hpp"
#include "gramma.h"
#include <set>

/**
* @brief constructor
*/
Search::Search(SearchOptions* search_opts, std::string word, Dict* dict) {
  search_options_ = search_opts;
  search_results_ = new std::map<std::string, double>;
  dict_ = dict;
  searched_word_ = word;
  std::cout << "SEARCHING FOR: " << searched_word_ << std::endl;
}

Search::~Search() {
    delete search_results_;
}


/**
* @brief calls spezific search function, searches, and creates map of  matches. Removes all 
* books that do not match with search options.
*/
std::map<std::string, double>* Search::search(map_words& mWs, map_words& mWsTitle, 
    map_words& mWsAuthor, map_books& mapBooks) {
  std::cout << "Searching for " << searched_word_ << "\n";

  //Normal search (full-match)
  if (search_options_->getFuzzyness() == false) {
    //Search in ocr and/ or in title
    if(search_options_->getOnlyTitle() == false)
      normalSearch(mWs);
    if(search_options_->getOnlyOcr() == false)
      normalSearch(mWsTitle);
    normalSearch(mWsAuthor);
  }

  //Fuzzy Search
  else {
    //Search in ocr and/ or in title
    if(search_options_->getOnlyTitle() == false)
      fuzzySearch(mWs, mapBooks, false);
    if (search_options_->getOnlyOcr() == false)
      fuzzySearch(mWsTitle, mapBooks, true);
    fuzzySearch(mWsAuthor, mapBooks, true);
  }

  //Check search-options and remove books from search results, that don't match
  removeBooks(mapBooks);

  return search_results_;
}

/**
* @brief search full-match
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void Search::normalSearch(map_words& mapWords) {
    std::string sInput = searched_word_;
  if(mapWords.count(searched_word_) > 0) {
    std::map<std::string, double> found = mapWords.at(searched_word_);
    search_results_->insert(found.begin(), found.end());
  }
}

/**
* @brief search fuzzy 
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void Search::fuzzySearch(map_words& mapWords, map_books& mapBooks, bool title) {
  for(auto it= mapWords.begin(); it!=mapWords.end(); it++) {
    double value = fuzzy::fuzzy_cmp(it->first, searched_word_);
    if(value <= 0.2 && title == false) 
      myInsert(it->second, it->first, mapBooks, value);
    else if(value <= 0.2)
      search_results_->insert(it->second.begin(), it->second.end());
  }
}

/**
* @brief inserts searchResults into map of searchresults and assigns value of match
* @param[out] mapSR
* @param[in] found
* @param[out] sMatch
* @param[in] value
*/
void Search::myInsert(std::map<std::string, double>& found, std::string sMatch, 
    map_books& mapBooks, double value) {
  for(auto it=found.begin(); it!=found.end(); it++) {
    (*search_results_)[it->first] += it->second*(1-value*5);

    //Add match to map
    if(mapBooks[it->first]->has_ocr() == false)
      continue;
    if (mapBooks[it->first]->found_fuzzy_matches()[searched_word_].front().second > value)
      mapBooks[it->first]->found_fuzzy_matches()[searched_word_].push_front({sMatch, value});
    else
      mapBooks[it->first]->found_fuzzy_matches()[searched_word_].push_back({sMatch, value});
  }
}

/**
* @brief remove all books that don't agree with searchOptions.
* @param[in, out] mapSR map of search results
*/
void Search::removeBooks(map_books& mapBooks) {
  for(auto it=search_results_->begin(); it!=search_results_->end();) {
    if(checkSearchOptions(mapBooks[it->first]) == false)
      it = search_results_->erase(it);
    else
      ++it;
  }
}

/**
* @param[in] book to be checked
* return Boolean
*/
bool Search::checkSearchOptions(Book* book) {
  //*** check ocr ***//
  if(search_options_->getOnlyOcr() == true && book->has_ocr() == false)
    return false;

  //*** check author ***//
  if(search_options_->getLastName().length() > 0) {
    if(book->author() != search_options_->getLastName())
      return false;
  }

  //*** check date ***//
  if(book->date()==-1 || book->date()<search_options_->getFrom() || book->date()>search_options_->getTo())
    return false;
       
  //*** check pillars ***//
  for(auto const &collection : search_options_->getCollections()) {
    if(func::in(collection, book->collections()) == true)
      return true;
  }
  return false;
}
