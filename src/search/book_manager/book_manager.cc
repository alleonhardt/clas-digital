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

std::list<ResultObject> BookManager::Search(SearchObject& search_object) {
  // Get converted words.
  std::vector<std::string> words = search_object.converted_words();
  bool fuzzy_search = search_object.search_options().fuzzy_search();
  
  // Caluculate start time.
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
  
  // Simplyfy results (convert to map of book-keys and relevance. 
  // Also add a pointer to the book for every result-object.
  // If search query contained more than one word, also manipulate relevance of
  // found match accoring to number of matches found on the same page.
  std::list<std::pair<double, std::string>> simplyfied_results;
  if (words.size() > 1) {
    for (auto& [key, res_obj] : results) {
      // Add book, and original words to result object and simplyfy result.
      res_obj.set_book(documents_.at(key));
      res_obj.set_original_words(search_object.converted_to_original());
      simplyfied_results.push_back({res_obj.score(), key});

      // Change score accoring to number of found words in one page.
      res_obj.set_score(res_obj.score()*
          res_obj.book()->WordsOnSamePage(res_obj.matches_as_list(), fuzzy_search));
    }
  }
  else {
    for (auto& [key, res_obj] : results) {
      // Add book, and original words to result object and simplyfy result.
      res_obj.set_book(documents_.at(key));
      res_obj.set_original_words(search_object.converted_to_original());
      simplyfied_results.push_back({res_obj.score(), key});
    }
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << "Completed search after " << elapsed_seconds.count() << " seconds." << std::endl;

  // Sort results results and convert to list of books.
  std::list<ResultObject> sorted_search_results;
  SortMapByValue(simplyfied_results, search_object.search_options().sort_results_by());
  for (auto it : simplyfied_results)
    sorted_search_results.push_back(results[it.second]); 

  end = std::chrono::system_clock::now();
  elapsed_seconds = end-start;
  std::cout << "Completed sorting after " << elapsed_seconds.count() << " seconds." << std::endl;

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
    std::map<std::string, ResultObject>& results) {
  // search in corpus: 
  for (const auto& it : index_list_) {

    // Calculate fuzzy score (occures in word, and levensthein-distance).
    int score = fuzzy::cmp(word, it.first);
    if (score == -1) 
      continue;

    // Iterate over all found items:
    for (auto item : it.second) {
      // Skip if item is in conflict with search-options. Only check 
      // search-options, if item is not already in results.
      if (results.count(item.first) == 0 && 
          !CheckSearchOptions(search_options, documents_[item.first])) 
        continue;
      // Add new information to result object.
      results[item.first].NewResult(word, it.first, item.second.scope_, score, item.second.relevance_, it.second.size());
    }
  }
}

bool BookManager::CheckSearchOptions(SearchOptions& search_options, Book* book) {
  // If an author is given and in the string of authors, the auther is not
  // found, return false.
  if(search_options.author().length() > 0) {
    if (book->authors().count(search_options.author()) == 0)
      return false;
  }

  // If date is specified (→ date is not -1) return false if date is outside of timerange.
  int date = book->date();
  if (date != -1 && (date < search_options.year_from() || date > search_options.year_to()))
    return false;
       
  // If no collections are specified in search options, return true
  if (search_options.collections().size() == 0)
    return true;
  for (const auto& collection : search_options.collections()) {
    if (book->collections().count(collection) > 0)
      return true;
  }
  return false;
}

void BookManager::SortMapByValue(sort_list& unordered_results, int sorting) {
  if (unordered_results.size() < 2) 
    return;

  // Call matching sorting algorythem.
  if (sorting == 0) return SortByRelavance(unordered_results);
  if (sorting == 1) return SortChronologically(unordered_results);
  if (sorting == 2) return SortAlphabetically(unordered_results);
}
  
void BookManager::SortByRelavance(sort_list& unordered) {
  unordered.sort([](const auto &a, const auto &b) {
        if (a.first == b.first) 
          return a.second > b.second;
        return a.first > b.first; 
    });
}

void BookManager::SortChronologically(sort_list& unordered) {
  unordered.sort([&](const auto &a, const auto &b) {
        int date1 = documents_[a.second]->date();
        int date2 = documents_[b.second]->date();
        if (date1 == date2) 
          return a.first < b.first;
        return date1 < date2; 
    });
}

void BookManager::SortAlphabetically(sort_list& unordered) {
  unordered.sort([&](const auto &a,const auto &b) {
        std::string author1 = *documents_[a.second]->authors().cbegin();
        std::string author2 = *documents_[b.second]->authors().cbegin();
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
      if (!documents_[jt.first]->HasContent())
        jt.second.relevance_ = okapi / 2;
      // Increase scope, if word was found in metadata & set relevance.
      else if (jt.second.scope_>1)
        jt.second.relevance_ = okapi * 2;
      else
        jt.second.relevance_ = okapi;
    }
  }

  // Create index map as list:
  for (const auto& it : index_map_)
    index_list_.push_back({it.first, it.second});
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
