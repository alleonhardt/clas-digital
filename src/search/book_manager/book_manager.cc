#include "book_manager.h"
#include "book_manager/book.h"
#include "func.hpp"
#include "gramma.h"
#include "nlohmann/json.hpp"
#include "result_object.h"
#include "search_object.h"
#include "search_options.h"
#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <numeric>
#include <ostream>
#include <utility>
#include <vector>

BookManager::BookManager(std::vector<std::string> mount_points, Dict* dict, const nlohmann::json& search_config) 
  : upload_points_(mount_points) {

  // Load dictionary
  full_dict_ = dict;

  // Set search-config and created metadata tags (bit → {tag-name, relevance}).
  search_config_ = search_config;
  metadata_tags_ = func::CreateMetadataTags(search_config_);  

  
  // Revert metadata_tags_ to easily find matching bit representation of string
  // (tag-name → bit)
  for (const auto& it : metadata_tags_) {
    std::cout << it.first << ": " << it.second.first << ", " << it.second.second << std::endl;
    reverted_tag_map_[it.second.first] = it.first;
  }

  // Set static variable for in books, to grant books acces to both
  // metadata-tags and their revert accessing map.
  Book::set_metadata_tag_reference(metadata_tags_);
  Book::set_reverted_tag_reference(reverted_tag_map_);
}

std::unordered_map<std::string, Book*>& BookManager::documents() {
  return documents_;
}

const BookManager::index_map_type& BookManager::index_map() const {
  return index_map_;
}

bool BookManager::Initialize(bool reload_pages) {
  std::cout << "BookManager: Extracting books." << std::endl;
  
  //Go though all and create item-index-map, only if book was created from metadata. 
  for (auto upload_point : upload_points_) {
    for (const auto& p : std::filesystem::directory_iterator(upload_point)) {
      std::string filename = p.path().stem().string();
      if (documents_.count(filename) > 0)
        AddBook(p.path(), filename, reload_pages);
    }
  }

  //Create map of all words + and of all words in all titles
  std::cout << "BookManager: Creating map of books." << std::endl;
  CreateIndexMap();
  std::cout << "Map words:   " << index_map_.size() << "\n";
  std::cout << "Scopes: " << std::endl;
  for (auto it : index_map_) {
    std::cout << it.first << ": " << std::endl;
    for (auto jt : it.second) {
      std::cout << "-- " << jt.first << ": " << jt.second.scope_ << " | " << jt.second.relevance_ << std::endl;
    }
  }

  // Create type-ahead lists of sorted words and sorted authors.
  CreateListWords(list_words_, 0);
  CreateListWords(list_authors_, reverted_tag_map_["authors"]);

  return true;
}

void BookManager::CreateItemsFromMetadata(nlohmann::json j_items) {
  // Convert items according to search_config.
  auto items = ConvertMetadata(j_items);

  //Iterate over all items in json
  for (auto &it : items) {
    std::string key = it[reverted_tag_map_["key"]];
    // already exists: update metadata.
    if (documents_.count(key) > 0)
      documents_[key]->UpdateMetadata(it);

    // does not exits: create new book and add to map of all books.
    else
      documents_[key] = new Book(it);
  }
}

void BookManager::AddBook(std::string path, std::string sKey, bool reload_pages) {
  if(!std::filesystem::exists(path))
    return;
  documents_[sKey]->InitializeBook(path, reload_pages);
}
    

std::list<ResultObject> BookManager::Search(SearchObject& search_object) {

  // Get converted words.
  std::vector<std::string> words = search_object.converted_words();
  
  // Caluculate elapsed seconds.
  auto start = std::chrono::system_clock::now();

  // Start first search:
  std::map<std::string, ResultObject> results;
  DoSearch(results, words[0], search_object.search_options());

  // Do search for every other word.
  for (size_t i=1; i<words.size(); i++) {

    // Start nth search:
    std::map<std::string, ResultObject> results2;
    DoSearch(results2, words[i], search_object.search_options());
 
    // Erase all words from results which were not found in new results and join
    // result-object-infos, of all others.
    for (auto it=results.begin(); it!=results.end();) {
      // Erase if not found.
      if (results2.count(it->first) == 0)
        it = results.erase(it);
      // Join result-object (found scope, map of words, with scope)
      else {
        it->second.Join(results2[it->first]); 
        ++it;
      }
    }
    if (results.size() == 0)
      return {};
  }

  // Check if words are all on the same page:
  if (words.size() > 1) {
    for (auto it=results.begin(); it!=results.end();) {
      it->second.set_book(documents_.at(it->first));
      if (!it->second.found_in_metadata() 
          && !it->second.book()->OnSamePage(words, search_object.search_options().fuzzy_search()))
        it = results.erase(it);
      else
        ++it;
    }
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << "Completed search after " << elapsed_seconds.count() << " seconds." << std::endl;

  // Simplyfy results (convert to map of book-keys and relevance. 
  // Also add a pointer to the book for every result-object.
  std::map<std::string, double> simplified_results;
  for (auto& it : results) {
    it.second.set_book(documents_.at(it.first));
    simplified_results[it.first] = it.second.score();
  }

  //Sort results results and convert to list of books.
  std::list<ResultObject> sorted_search_results;
  for (auto it : SortMapByValue(simplified_results, search_object.search_options().sort_results_by())) {
    sorted_search_results.push_back(results.at(it.first)); 
  }

  auto end2 = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds2 = end2-start;
  std::cout << "Completed search and sorting after " << elapsed_seconds2.count() << " seconds." << std::endl;

  return sorted_search_results;
}

void BookManager::DoSearch(std::map<std::string, ResultObject> &results, std::string word, 
    SearchOptions &search_options) {
  if (search_options.fuzzy_search())
    FuzzySearch(word, search_options, results);
  else
   NormalSearch(word, search_options, results);
}

void BookManager::NormalSearch(std::string word, SearchOptions& search_options, 
    std::map<std::string, ResultObject>& results) {
  // Get all books, which contain the searched word.
  std::map<std::string, MatchObject> tmp = index_map_[word];

  // For each found book, whether it matches with the search-options. If so, add
  // new-search-result.
  for (const auto& it : tmp) {
    if (CheckSearchOptions(search_options, documents_[it.first]))
      results[it.first].NewResult(word, it.second.scope_, it.second.relevance_);
  }
}

void BookManager::FuzzySearch(std::string word, SearchOptions& search_options, 
    std::map<std::string, ResultObject>& results) {
  // search in corpus: 
  for (const auto& it : index_list_) {
    double value = fuzzy::fuzzy_cmp(it.first, word);
    if (value > 0.2) 
      continue;
    // Iterate over all found items:
    for (auto item : it.second) {
      // Skip if item is in conflict with search-options.
      if (!CheckSearchOptions(search_options, documents_[item.first])) 
        continue;
      // Add new information to result object.
      results[item.first].NewResult(word, item.second.scope_, item.second.relevance_);
      
      // Add found match to list of fuzzy matches found for searched word.
      if (item.second.scope_ & 1)
        documents_[item.first]->corpus_fuzzy_matches()[word].Insert({it.first, value});
      else 
        documents_[item.first]->metadata_fuzzy_matches()[word].Insert({it.first, value});
    }
  }
}

bool BookManager::CheckSearchOptions(SearchOptions& search_options, Book* book) {
  // author:
  if(search_options.author().length() > 0) {
    if(book->GetFromMetadata("author") != search_options.author())
      return false;
  }

  int date = book->date();
  if (date == -1 || date < search_options.year_from() || date > search_options.year_to())
    return false;
       
  // collections:
  if (search_options.collections().size() == 0)
    return true;
  for (auto const &collection : search_options.collections()) {
    if (func::in(collection, book->collections()))
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
        int date1 = stoi(documents_[a.first]->GetFromMetadata("date"));
        int date2 = stoi(documents_[b.first]->GetFromMetadata("date"));
        if(date1==date2) 
          return a.first < b.first;
        return date1 < date2; });
  return sorted_results;
}

BookManager::sorted_set BookManager::SortAlphabetically(std::map<std::string, double> unordered) {
	sorted_set sorted_results(unordered.begin(), unordered.end(), [&](const auto &a,const auto &b) {
        std::string author1 = documents_[a.first]->GetFromMetadata("description");
        std::string author2 = documents_[b.first]->GetFromMetadata("description");
        if(author1 == author2) 
          return a.first < b.first;
        return author1 < author2 ; } );
  return sorted_results;
}

void BookManager::CreateIndexMap() {

  int num_documents = documents().size();
  double avglength = 0;
  for (const auto& it : documents_)
    avglength += it.second->GetLength();
  avglength /= num_documents;


  // Create main-index map from all 
  for (auto it : documents_) {
    AddWordsFromItem(it.second->words_on_pages(), true, it.first);
    AddWordsFromItem(it.second->words_in_tags(), false, it.first);
  }

  // Calculate Okapi
  for (auto& it : index_map_) {
    
    // Caluculate Inverse document frequency (idenical for each entry (word, document))
    size_t n_q = it.second.size(); // number of documents containing current word.
    double idf = std::log((num_documents-n_q+0.5)/(n_q+0.5)+1); 

    // Caluculate document specific Part
    for (auto& jt : it.second) {
      double tf = jt.second.relevance_; // term frequency.
      size_t len_document = documents_[jt.first]->GetLength();
      double okapi = idf + ((tf*(2.0+1.0))/(tf+2.0*(1-0.75+0.75*(len_document/avglength))));

      // Decrease scope if document does not have a corpus.
      if (!documents_[jt.first]->HasContent())
        okapi /= 2;
      // Increase scope, if word was found in metadata.
      else if (jt.second.scope_>1)
        okapi *= 2;
      
      // Set new relevance of this match object.
      jt.second.relevance_ = okapi;
    }
  }

  // Create index map as list:
  for (const auto& it : index_map_) {
    index_list_.push_back({it.first, it.second});
  }
}

void BookManager::AddWordsFromItem(std::unordered_map<std::string, std::vector<WordInfo>> m, 
    bool corpus, std::string item_key) {
  for (auto it : m) {
    // Calculate total relevance
    index_map_[it.first][item_key].relevance_ = std::accumulate(it.second.begin(), it.second.end(), 0.0, 
        [](const double& init, const auto& word_info) { return (init+word_info.term_frequency_); });

    // If found in corpus, add corpus to scope. (corpus→1), metadata→get all different tags.
    if (corpus)
      index_map_[it.first][item_key].scope_ |= 1;
    // If found in metadata, add all different metadata-tags (2,4,8, ...). 
    else {
      for (const auto& word_info : it.second) {
        for (const auto& scope : word_info.pages_)
          index_map_[it.first][item_key].scope_ |= scope;
      }
    }
  }
}

void BookManager::CreateListWords(sorted_list_type& list_of_words, short scope) {
  std::map<std::string, double> map_word_relevance;
  // Transform into above map type.
  for (auto it = index_map_.begin(); it!=index_map_.end(); it++) {
    
    // Get complete scope -> where does this word occure
    short cur_scope = std::accumulate(it->second.begin(), it->second.end(), 0, 
        [](const short& init, const auto& elem) { return init|elem.second.scope_; });

    // Check wether scope matches given scope.
    if (scope == 0 || cur_scope&scope) 
      map_word_relevance[it->first] = it->second.size();
  }

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

std::list<std::string> BookManager::GetSuggestions(std::string word, sorted_list_type& list_words) {
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

std::vector<std::map<short, std::string>> BookManager::ConvertMetadata(const nlohmann::json &metadata_items) {
  std::cout << "ConvertMetadata" << std::endl;
  // Convert each item in metadata.
  std::vector<std::map<short, std::string>> items;
  for (auto item : metadata_items) {
    std::map<short, std::string> key_converted;
    // Convert key to bit-representation.
    for (const auto& it : func::ConvertJson(item, search_config_))
      key_converted[reverted_tag_map_[it.first]] = it.second; 
    items.push_back(key_converted);
  }
  return items;
}
