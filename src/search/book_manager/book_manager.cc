#include "book_manager.h"
#include "book_manager/book.h"
#include "func.hpp"
#include "gramma.h"
#include "nlohmann/json.hpp"
#include "result_object.h"
#include "search_object.h"
#include "search_options.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <numeric>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

BookManager::BookManager(std::vector<std::string> mount_points, Dict* dict, 
    const nlohmann::json& search_config, std::string search_data_location) 
  : db_(Database(search_data_location)),  upload_points_(mount_points){

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
  Book::set_datebase(&db_);
}

std::unordered_map<std::string, Book*>& BookManager::documents() {
  return documents_;
}

const BookManager::index_map_type& BookManager::index_map() const {
  return index_map_;
}

bool BookManager::Initialize(bool reload_pages) {
  std::cout << "BookManager: Extracting books." << std::endl;
  
  // Go though all and create item-index-map, only if book was created from metadata. 
  for (auto upload_point : upload_points_) {
    for (const auto& p : std::filesystem::directory_iterator(upload_point)) {
      std::string filename = p.path().stem().string();
      if (documents_.count(filename) > 0)
        AddBook(p.path(), filename, reload_pages);
    }
  }

  // Afer Initializing all books is done, the pages need to be transmitted to
  // the database.
  db_.AddPages();

  //Create map of all words + and of all words in all titles
  std::cout << "BookManager: Creating map of books." << std::endl;
  CreateIndexMap();
  std::cout << "Map words: " << index_map_.size() << "\n";

  size_t words_with_base_form = 0;
  size_t base_forms = 0;
  int counter=0;
  for (const auto& book : documents_) {
    for (const auto& jt : documents_[book.first]->words_on_pages()) {
      words_with_base_form += jt.second.size();
      base_forms++;
    }
  }
  std::cout << "Total baseforms: " << base_forms << std::endl;
  std::cout << "Baseforms with conjunctions: " << words_with_base_form << std::endl;

  // Create type-ahead lists of sorted words and sorted authors.
  CreateListWords(list_words_, 0);
  nlohmann::json all_words = list_words_;
  func::WriteContentToDisc("all_words.json", all_words);
  std::cout << "List words: " << list_words_.size() << "\n";

  CreateListWords(list_authors_, reverted_tag_map_["authors"]);

  return true;
}

void BookManager::CreateItemsFromMetadata(nlohmann::json j_items, bool reload_pages) {
  // Empty complete database, when reloading pages is set.
  if (reload_pages)
    db_.ClearDatabase();

  // Convert items according to search_config.
  std::cout << "Converting items." << std::endl;
  auto items = ConvertMetadata(j_items);

  //Iterate over all items in json
  for (auto &it : items) {
    nlohmann::json json = it;
    std::string key = it[reverted_tag_map_["key"]];
    // already exists: update metadata.
    if (documents_.count(key) > 0)
      documents_[key]->UpdateMetadata(it);

    // does not exits: create new book and add to map of all books.
    else
      documents_[key] = new Book(it);

    // Add document to database (only queued).
    db_.AddDocument(key);
  }

  // Submit all queued documents.
  db_.AddDocuments();
}

void BookManager::AddBook(std::string path, std::string key, bool reload_pages) {
  if(!std::filesystem::exists(path))
    return;
  documents_[key]->InitializeBook(path, reload_pages);
}

std::list<ResultObject> BookManager::Search(SearchObject& search_object, int limit) {
  if (search_object.query() == "")
    return {};

  // Get all words containing all entered terms.
  auto start = std::chrono::system_clock::now();
  std::map<std::string, ResultObject> results;
  SearchNWords(results, search_object, limit);
  
  // Print time needed for searching and preparing results.
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << "Completed search after " << elapsed_seconds.count() << " seconds." << std::endl;

  // Simplyfy results (convert to map of book-keys and relevance. 
  // Also add a pointer to the book for every result-object.
  // If search query contained more than one word, also manipulate relevance of
  // found match accoring to number of matches found on the same page.
  prepared_results_type prepared_results;
  PrepareResults(prepared_results, results, search_object);
  
  // Print time needed for searching and preparing results.
  end = std::chrono::system_clock::now();
  elapsed_seconds = end-start;
  std::cout << "Completed preparing after " << elapsed_seconds.count() << " seconds." << std::endl;

  // Sort results results and convert to list of books.
  std::list<ResultObject> sorted_search_results;
  SortMapByValue(prepared_results, search_object.search_options().sort_results_by());
  for (auto it : prepared_results)
    sorted_search_results.push_back(results[it.second]); 

  // Print time needed for searching, preparing and sorting.
  end = std::chrono::system_clock::now();
  elapsed_seconds = end-start;
  std::cout << "Completed sorting after " << elapsed_seconds.count() << " seconds." << std::endl;

  return sorted_search_results;
}

void BookManager::SearchNWords(std::map<std::string, ResultObject> &results, SearchObject &search_object,
    int limit) {
  // Get converted words.
  std::vector<std::string> words = search_object.converted_words();

  // Search for first word.
  SearchOneWord(results, words[0], search_object.search_options(), limit);

  // Do search for every other word.
  for (size_t i=1; i<words.size(); i++) {
    // Start nth search:
    std::map<std::string, ResultObject> results2;
    SearchOneWord(results2, words[i], search_object.search_options(), limit);
 
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
      return;
  }
}

void BookManager::SearchOneWord(std::map<std::string, ResultObject> &results, std::string word, 
    SearchOptions &search_options, int limit) {
  if (search_options.fuzzy_search()) {
    FuzzySearch(word, search_options, results, index_list_);
    std::cout << "Results: " << results.size();
    if (results.size() < limit) {
      FuzzySearch(word, search_options, results, index_list_b_);
      std::cout << "Results: " << results.size();
    }
  }
  else
    NormalSearch(word, search_options, results);
}

void BookManager::NormalSearch(std::string word, SearchOptions& search_options, 
    std::map<std::string, ResultObject>& results) {
  // Get all books, which contain the searched word.
  std::map<std::string, Match> tmp; 
  try {
    tmp = index_map_.at(word);
  } catch(...) {
    return;
  }

  // For each found book, whether it matches with the search-options. If so, add
  // new-search-result.
  for (const auto& it : tmp) {
    if (CheckSearchOptions(search_options, documents_[it.first]))
      results[it.first].NewResult(word, word, it.second.scope_, 0, it.second.relevance_);
  }
}

void BookManager::FuzzySearch(std::string word, SearchOptions& search_options, 
    std::map<std::string, ResultObject>& results, index_list_type& index_list) {
  // search in corpus: 
  for (const auto& array : index_list) {
    for (const auto& it : array) {
      // Calculate fuzzy score (occures in word, and levensthein-distance).
      int score = fuzzy::cmp(word, it);
      if (score == -1) 
        continue;

      // Iterate over all found items:
      auto matching_items = index_map_[it];
      for (auto item : matching_items) {
        // Skip if item is in conflict with search-options. Only check 
        // search-options, if item is not already in results.
        if (results.count(item.first) == 0 && !CheckSearchOptions(search_options, documents_[item.first])) 
          continue;
        // Add new information to result object.
        results[item.first].NewResult(word, it, item.second.scope_, score, 
            item.second.relevance_, matching_items.size());
      }
    }
  }
}

bool BookManager::CheckSearchOptions(SearchOptions& search_options, Book* document) {
  
  // If only metadata, but document has a corpus return false
  if (search_options.only_metadata() && document->has_ocr())
    return false;

  // If only corpus, but document does not have a corpus, return false
  if (search_options.only_corpus() && !document->has_ocr())
    return false;
  
  // If an author is given and in the string of authors, the auther is not
  // found, return false.
  if (search_options.author().length() > 0) {
    if (document->authors().count(search_options.author()) == 0)
      return false;
  }

  // If date is specified (→ date is not -1) return false if date is outside of timerange.
  int date = document->date();
  if (date != -1 && (date < search_options.year_from() || date > search_options.year_to()))
    return false;
       
  // If no collections are specified in search options, return true
  if (search_options.collections().size() == 0)
    return true;
  for (const auto& collection : search_options.collections()) {
    if (document->collections().count(collection) > 0)
      return true;
  }
  return false;
}

void BookManager::PrepareResults(prepared_results_type &prepared_results, std::map<std::string, ResultObject> &results,
    SearchObject& search_object) {

  auto converted_to_original = search_object.converted_to_original();
  bool fuzzy = search_object.search_options().fuzzy_search();

  // Simplyfy results (convert to map of book-keys and relevance. 
  // Also add a pointer to the book for every result-object.
  // If search query contained more than one word, also manipulate relevance of
  // found match accoring to number of matches found on the same page.
  if (search_object.converted_words().size() > 1) {
    for (auto& [key, res_obj] : results) {
      // Add book, and original words to result object and simplyfy result.
      res_obj.set_book(documents_.at(key));
      res_obj.set_original_words(converted_to_original);
      prepared_results.push_back({res_obj.score(), key});

      // Change score accoring to number of found words in one page.
      res_obj.set_score(res_obj.score()*
          res_obj.book()->WordsOnSamePage(res_obj.matches_as_list(), fuzzy));
    }
  }
  else {
    for (auto& [key, res_obj] : results) {
      // Add book, and original words to result object and simplyfy result.
      res_obj.set_book(documents_.at(key));
      res_obj.set_original_words(converted_to_original);
      prepared_results.push_back({res_obj.score(), key});
    }
  }
}

void BookManager::SortMapByValue(prepared_results_type& unordered_results, int sorting) {
  if (unordered_results.size() < 2) 
    return;

  // Call matching sorting algorythem.
  if (sorting == 0) return SortByRelavance(unordered_results);
  if (sorting == 1) return SortChronologically(unordered_results);
  if (sorting == 2) return SortAlphabetically(unordered_results);
}
  
void BookManager::SortByRelavance(prepared_results_type& unordered) {
  unordered.sort([](const auto &a, const auto &b) {
        if (a.first == b.first) 
          return a.second > b.second;
        return a.first > b.first; 
    });
}

void BookManager::SortChronologically(prepared_results_type& unordered) {
  unordered.sort([&](const auto &a, const auto &b) {
        int date1 = documents_[a.second]->date();
        if (date1 == -1) date1 = 2050;
        int date2 = documents_[b.second]->date();
        if (date2 == -1) date2 = 2050;
        if (date1 == date2) 
          return a.first < b.first;
        return date1 < date2; 
    });
}

void BookManager::SortAlphabetically(prepared_results_type& unordered) {
  unordered.sort([&](const auto &a,const auto &b) {
        std::string author1 = documents_[a.second]->first_author_lower();
        std::string author2 = documents_[b.second]->first_author_lower();
        if (author1 == author2) 
          return a.first < b.first;
        return author1 < author2 ; 
    });
}

void BookManager::CreateIndexMap() {

  int num_documents = documents().size();
  double avglength = 0;
  for (const auto& it : documents_)
    avglength += it.second->document_size();
  avglength /= num_documents;

  std::cout << "Number of documents: " << num_documents << std::endl;
  std::cout << "Average length of documents: " << avglength << std::endl;

  // Create main-index map from all 
  for (auto it : documents_) {
    AddWordsFromItem(it.second->words_on_pages(), true, it.first);
    AddWordsFromItem(it.second->words_in_tags(), false, it.first);
  }

  // Calculate Okapi
  int counter=0;
  for (auto& it : index_map_) {
    
    // Caluculate Inverse document frequency (idenical for each entry (word, document))
    size_t n_q = it.second.size(); // number of documents containing current word.
    double idf = std::log((num_documents-n_q+0.5)/(n_q+0.5)+1); 

    // Caluculate document specific Part
    for (auto& jt : it.second) {
      double tf = jt.second.relevance_; // term frequency.
      size_t len_document = documents_[jt.first]->document_size();
      double okapi = idf * ((tf*(2.0+1.0))/(tf+2.0*(1-0.75+0.75*(len_document/avglength))));

      // Decrease scope if document does not have a corpus & set relevance.
      if (!documents_[jt.first]->has_ocr())
        jt.second.relevance_ = okapi / 4;
      // Increase scope, if word was found in metadata & set relevance.
      else if (jt.second.scope_>1)
        jt.second.relevance_ = okapi * 2;
      else
        jt.second.relevance_ = okapi;
    }
  }

  // Create index map as list:
  CreateIndexList(index_list_, 5, true);  // primary list with all words occuring in more than 5 documents.
  CreateIndexList(index_list_b_, 5, false);  // secondary list with all words with low occurance.

  std::cout << "List words: " << index_list_.size() << std::endl;
  std::cout << "List words: " << index_list_b_.size() << std::endl;
}

void BookManager::CreateIndexList(index_list_type& index_list, int occurance, bool primary) {
  // Create index map as list:
  int word_count = 0;
  std::array<std::string, 100000> tmp; // temporary array.
  for (const auto& it : index_map_) {
    // If primary add only words occuring > occurance times, other wise less that
    if ((primary && it.second.size() >= occurance) || (!primary && it.second.size() < occurance))
      tmp[word_count++] = it.first;

    // If array-size is reached add array to index-list, empty array and reset counter.
    if (word_count > 99998) {
      word_count = 0;
      index_list.push_back(tmp);
      std::cout << "Added: " << tmp.empty() << std::endl;
    }
  }
  index_list.push_back(tmp);
}

void BookManager::AddWordsFromItem(std::unordered_map<std::string, std::vector<WordInfo>> m, 
    bool corpus, std::string item_key) {
  for (auto it : m) {
    // Calculate total relevance
    index_map_[it.first][item_key].relevance_ = std::accumulate(it.second.begin(), it.second.end(), 
        0.0, [](const double& init, const auto& word_info) { 
        return (init+word_info.term_frequency_); 
      });

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

    // Check wether scope matches given scope and insert. 
    // RELEVANCE: number of documents containing this word.
    if (scope == 0 || cur_scope&scope) 
      map_word_relevance[it->first] = it->second.size();
  }

  for (auto elem : func::SortByRelavance(map_word_relevance))
    list_of_words.push_back({elem.first, elem.second});
}


std::list<std::string> BookManager::GetSuggestions(std::string word, std::string scope) {
  if (scope == "corpus") 
    return GetSuggestions(word, list_words_);
  if (scope == "author") 
    return GetSuggestions(word, list_authors_);
  return std::list<std::string>();
}

std::list<std::string> BookManager::GetSuggestions(std::string word, sorted_list_type& list_words) {
  std::cout << "GetSuggestions" << std::endl;
  // Replace multiple byte characters and convert to lowercase.
  word = func::ReplaceMultiByteChars(func::ReturnToLower(word));

  // Create list of pairs, with the resulting relevance as first value (for sorting).
  std::list<std::pair<double, std::string>> suggs;
  size_t counter = 0; 
  for (const auto& it : list_words) {
    // calculate levensthein.
    double value = fuzzy::contains(word.c_str(), it.first.c_str(), word.length(), it.first.length()); 
    
    // Only add if found (0: direct match, 1,2: fuzzy or contains match)
    if(value != -1) {
      suggs.push_back({(1.0/(1.0+value))*it.second, it.first});
      
      // If 50 rersults where found, stop searching. This usually only makes a
      // difference for short, and unfinished words, the user won't feel a
      // difference, but the search time is reduced to upto 10 times (for
      // example when search for single letters.
      if (++counter >= 50)
        break;
    }
  }

  // Sort list.
  SortByRelavance(suggs);
  
  // Take the best 10 matches and return in expected list type.
  std::list<std::string> sorted_search_results;
  counter=0;
  for (auto it : suggs) {
    sorted_search_results.push_back(it.second); 
    if (++counter == 10) break;
  }
  return sorted_search_results;
}

std::string BookManager::GetConjunction(std::string suggestion) {
  // Get First book to contain this word.
  Book* book = documents_[index_map_[suggestion].begin()->first];
  if (book->words_on_pages().count(suggestion) > 0)
    return book->words_on_pages().at(suggestion).begin()->word_;
  else
   return book->words_in_tags().at(suggestion).begin()->word_;
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
