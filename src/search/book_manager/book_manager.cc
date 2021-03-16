#include "book_manager.h"
#include "book_manager/book.h"
#include "func.hpp"
#include "gramma.h"
#include "result_object.h"
#include "search_object.h"
#include "search_options.h"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>

BookManager::BookManager(std::vector<std::string> mount_points, Dict* dict) 
  : upload_points_(mount_points){
  std::string sBuffer;
  full_dict_ = dict;
}

std::unordered_map<std::string, Book*>& BookManager::map_of_books() {
  return map_books_;
}

BookManager::MAPWORDS& BookManager::map_of_authors() {
  return map_words_authors_;
}

std::map<std::string, std::vector<std::string>>& BookManager::map_unique_authors() {
  return map_unique_authors_;
}

bool BookManager::Initialize(bool reload_pages) {
  std::cout << "BookManager: Extracting books." << std::endl;
  //Go though all books and create book
  for (auto upload_point : upload_points_) {
    for (const auto& p : std::filesystem::directory_iterator(upload_point)) {
      std::string filename = p.path().stem().string();
      if (map_books_.count(filename) > 0)
        AddBook(p.path(), filename, reload_pages);
    }
  }

  //Create map of all words + and of all words in all titles
  std::cout << "BookManager: Creating map of books." << std::endl;
  CreateMapWords();
  CreateMapWordsTitle();
  CreateMapWordsAuthor();
  std::cout << "Map words:   " << map_words_.size() << "\n";
  std::cout << "Map title:   " << map_words_title_.size() << "\n";
  std::cout << "Map authors: " << map_words_authors_.size() << "\n";
  
  return true;
}

void BookManager::CreaeItemsFromMetadata(nlohmann::json j_items) {
  //Iterate over all items in json
  for (auto &it : j_items) {
    // already exists: update metadata.
    if (map_books_.count(it["key"]) > 0)
      map_books_[it["key"]]->metadata().set_json(it);

    // does not exits: create new book and add to map of all books.
    else
      map_books_[it["key"]] = new Book(it);

    // If book's json does not contain the most relevant information, delete again
    if(map_books_[it["key"]]->metadata().CheckJsonSet() == false) {
      delete map_books_[it["key"]];
      map_books_.erase(it["key"]);
    }
  }
}

void BookManager::AddBook(std::string path, std::string sKey, bool reload_pages) {
  if(!std::filesystem::exists(path))
    return;
  map_books_[sKey]->InitializeBook(path, reload_pages);
}
    

std::list<ResultObject> BookManager::DoSearch(SearchObject& search_object) {
  std::vector<std::string> words = search_object.converted_words();

  // Start first search:
  std::map<std::string, ResultObject> results;
  if (search_object.search_options().fuzzy_search())
    FuzzySearch(words[0], search_object.search_options(), results);
  else
    NormalSearch(words[0], search_object.search_options(), results);

  // Do search for every other word.
  size_t counter = 1;
  for (size_t i=1; i<words.size(); i++) {

    // Start first search:
    std::map<std::string, ResultObject> results2;
    if (search_object.search_options().fuzzy_search())
      FuzzySearch(words[0], search_object.search_options(), results2);
    else
      NormalSearch(words[0], search_object.search_options(), results2);
 
    for (auto it=results.begin(); it!=results.end();) {
      if (results2.count(it->first) == 0)
        it = results.erase(it);
      else if (!it->second.found_in_metadata() 
          && it->second.book()->OnSamePage(words, 
            search_object.search_options().fuzzy_search())==false)
        it = results.erase(it);
      else
        ++it;
    }
    counter++;
    if (results.size() == 0)
      return {};
  }
  //Sort results results and convert to list of books.
  std::map<std::string, double> simplified_results;
  for (auto it : results)
    simplified_results[it.first] = it.second.GetOverallScore();
  std::list<ResultObject> sorted_search_results;
  for (auto it : SortMapByValue(simplified_results, search_object.search_options().sort_results_by()))
    sorted_search_results.push_back(results.at(it.first)); 
  return sorted_search_results;
}

void BookManager::NormalSearch(std::string word, SearchOptions& search_options, 
    std::map<std::string, ResultObject>& results) {
  // Search in corpus:
  std::map<std::string, double> tmp = map_words_[word];
  for (auto it : tmp) {
    if (CheckSearchOptions(search_options, map_books_[it.first]))
      results[it.first] = ResultObject(map_books_[it.first], true, it.second);
  }

  // Search in metadata:
  tmp = map_words_title_[word];
  for (auto it : tmp) {
    if (results.count(it.first) > 0)
      results[it.first].FoundInMetadataSetInitScore(it.second);
    else
      results[it.first] = ResultObject(map_books_[it.first], false, it.second);
  }
}

void BookManager::FuzzySearch(std::string word, SearchOptions& search_options, 
    std::map<std::string, ResultObject>& results) {
  // search in corpus: 
  for (auto it : map_words_) {
    double value = fuzzy::fuzzy_cmp(it.first, word);
    if (value > 0.2) 
      continue;
    // Iterate over all found books:
    for (auto book : it.second) {
      // Skip if book is in conflict with search-options.
      if (!CheckSearchOptions(search_options, map_books_[book.first])) 
        continue;
      // if book already exists in results -> increase score.
      if (results.count(book.first) > 0) 
        results[book.first].IncreaseCorpusScore(book.second);
      // create result-object otherwise.
      else 
        results[book.first] = ResultObject(map_books_[book.first], true, book.second);

      // Add found match to list of fuzzy matches found for searched word.
      map_books_[book.first]->corpus_fuzzy_matches()[word].Insert({it.first, value});
    }
  }
  
  // search in metadata: 
  for (auto it : map_words_title_) {
    double value = fuzzy::fuzzy_cmp(it.first, word);
    if (value > 0.2) 
      continue;
    // Iterate over all found books:
    for (auto book : it.second) {
      // Skip if book is in conflict with search-options.
      if (!CheckSearchOptions(search_options, map_books_[book.first])) 
        continue;
      // if book already exists in results -> increase score.
      if (results.count(book.first) > 0 && results[book.first].found_in_metadata()) 
        results[book.first].IncreaseMetadataScore(book.second);
      // mark as found in metadata and set init metadata-score otherwise.
      else if (results.count(book.first) > 0 && !results[book.first].found_in_metadata()) 
        results[book.first].FoundInMetadataSetInitScore(book.second);
      // create result-object if non of the above.
      else
        results[book.first] = ResultObject(map_books_[book.first], false, book.second);
      // Add found match to list of fuzzy matches found for searched word.
      map_books_[book.first]->metadata_fuzzy_matches()[word].Insert({it.first, value});
    }
  }
}

bool BookManager::CheckSearchOptions(SearchOptions& search_options, Book* book) {
  // author:
  if(search_options.author().length() > 0) {
    if(book->author() != search_options.author())
      return false;
  }

  // date:
  if(book->date() == -1 
      || book->date() < search_options.year_from() 
      || book->date() > search_options.year_to())
    return false;
       
  // collections:
  for(auto const &collection : search_options.collections()) {
    if(func::in(collection, book->collections()))
      return true;
  }
  return false;
}

BookManager::sorted_set BookManager::SortMapByValue(
    std::map<std::string, double> unordered_results, int sorting) {
  if (unordered_results.size() == 0) 
    return sorted_set();
  if (unordered_results.size() == 1)
    return {std::make_pair(unordered_results.begin()->first, unordered_results.begin()->second)};

  // Call matching sorting algorythem.
  if (sorting == 0) return SortByRelavance(unordered_results);
  if (sorting == 1) return SortChronologically(unordered_results);
  if (sorting == 2) return SortAlphabetically(unordered_results);
  return sorted_set();
}
  
BookManager::sorted_set BookManager::SortByRelavance(std::map<std::string, double> unordered) {
	sorted_set sorted_results(unordered.begin(), unordered.end(), [](const auto &a,const auto &b) {
        if(a.second == b.second) 
          return a.first > b.first;
        return a.second > b.second; } );
  return sorted_results;
}

BookManager::sorted_set BookManager::SortChronologically(std::map<std::string, double> unordered) {
	sorted_set sorted_results(unordered.begin(), unordered.end(), [&](const auto &a,const auto &b) {
        int date1 = map_books_[a.first]->date();
        int date2 = map_books_[b.first]->date();
        if(date1==date2) 
          return a.first < b.first;
        return date1 < date2; });
  return sorted_results;
}

BookManager::sorted_set BookManager::SortAlphabetically(std::map<std::string, double> unordered) {
	sorted_set sorted_results(unordered.begin(), unordered.end(), [&](const auto &a,const auto &b) {
        std::string author1 = map_books_[a.first]->author();
        std::string author2 = map_books_[b.first]->author();
        if(author1 == author2) 
          return a.first < b.first;
        return author1 < author2 ; } );
  return sorted_results;
}

void BookManager::CreateMapWords() {
  // Iterate over all books. Add words to list and book to list of books with this word.
  for (auto it=map_books_.begin(); it!=map_books_.end(); it++) {
    // Check whether book has ocr.
    if(it->second->has_ocr() == false)
      continue;
    //Iterate over all words in book and add word to map and book to list with score.
    for(auto yt : it->second->map_words_pages()) {
      double total_relevance = 0;
      for (auto word_info : it->second->map_words_pages()[yt.first])
        total_relevance += word_info.relevance_;
      map_words_[yt.first][it->first] = total_relevance;
    }
  }
  CreateListWords(map_words_, list_words_);
} 

void BookManager::CreateMapWordsTitle() {
  // Iterate over all books. Add words from title to map.
  for (auto it=map_books_.begin(); it!=map_books_.end(); it++) {
    // Get map of words of current book.
    std::string sTitle = it->second->metadata().GetTitle();
    sTitle = func::convertStr(sTitle);
    std::map<std::string, int> mapWords = func::extractWordsFromString(sTitle);
    
    //Iterate over all words in this book. Check whether word already exists in list off all words.
    for (auto yt=mapWords.begin(); yt!=mapWords.end(); yt++) {
      std::string word = yt->first;
      map_words_title_[func::returnToLower(word)][it->first] = 0.1;
    }
    // Add author names and year.
    map_words_title_[std::to_string(it->second->date())][it->first] = 0.1;
  }
}

void BookManager::CreateMapWordsAuthor() {
  //Iterate over all books. Add words to list (if the word does not already exist 
  // in map of all words) or add book to list of books (if word already exists).
  for (auto it=map_books_.begin(); it!=map_books_.end(); it++) {
    for (auto author : it->second->metadata().GetAuthorsKeys()) {
      if (!it->second->metadata().IsAuthorEditor(author["creator_type"]))
        continue;
      map_words_authors_[func::returnToLower(author["lastname"])][it->first] = 0.1;
      map_unique_authors_[author["key"]].push_back(it->first);
    }
  }
  CreateListWords(map_words_authors_, list_authors_);
}

void BookManager::CreateListWords(MAPWORDS& mapWords, sortedList& list_of_words) {
  std::map<std::string, double> map_word_relevance;
  for (auto it = mapWords.begin(); it!=mapWords.end(); it++)
    map_word_relevance[it->first] = it->second.size();

  for (auto elem : SortByRelavance(map_word_relevance))
    list_of_words.push_back({elem.first, elem.second});
}


std::list<std::string> BookManager::GetSuggestions(std::string word, std::string scope) {
  if(scope=="corpus") 
    return GetSuggestions(word, list_words_);
  if(scope=="author") 
    return GetSuggestions(word, list_authors_);
  return std::list<std::string>();
}

std::list<std::string> BookManager::GetSuggestions(std::string word, sortedList& list_words) {
  std::cout << "GetSuggestions" << std::endl;
  word = func::convertStr(func::returnToLower(word));
  std::map<std::string, double> suggs;
  size_t counter=0; // TODO (fux) Check if we could generate more matches.
  for (auto it=list_words.begin(); it!=list_words.end() && ++counter <= 10; it++) {
    double value = fuzzy::fuzzy_cmp(it->first, word);
    if(value > 0 && value <= 0.2)
      suggs[it->first] = value*(-1);
  }
  std::list<std::string> sorted_search_results;
  for (auto it : SortByRelavance(suggs)) 
    sorted_search_results.push_back(it.first); 
  return sorted_search_results;
}
