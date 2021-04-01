/**
* @author fux
*/

#ifndef CLASDIGITAL_SRC_SEARCH_BOOKMANAGER_H_
#define CLASDIGITAL_SRC_SEARCH_BOOKMANAGER_H_

#include <algorithm> 
#include <filesystem>
#include <functional> 
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

#include "book.h"
#include "func.hpp"
#include "gramma.h"
#include "match_object.h"
#include "result_object.h"
#include "search_object.h"
#include "search_options.h"


class BookManager {
  private:
    Dict* full_dict_;
    const std::vector<std::string> upload_points_; ///< mount_points for book-locations.
    nlohmann::json search_config_;
    std::map<short, std::pair<std::string, double>> metadata_tags_;
    std::map<std::string, short> reverted_tag_map_;

    std::unordered_map<std::string, Book*> documents_; ///< map of all books.

    // Map of words / map of words in titles
    typedef std::unordered_map<std::string, std::map<std::string, MatchObject>> index_map_type;
    index_map_type index_map_;
    std::vector<std::pair<std::string, std::map<std::string, MatchObject>>> index_list_;

    typedef std::list<std::pair<std::string, size_t>> sorted_list_type;
    sorted_list_type list_words_; ///< Sorted list of all words by score (for typeahead).
    sorted_list_type list_authors_; ///< Sorted list of all authors by score (for typeahead). 

    std::shared_mutex search_lock_;

  public:
    ///< Constructor
    BookManager(std::vector<std::string> paths_to_books, Dict* dict, const nlohmann::json& search_config);

    // **** getter **** //

    std::unordered_map<std::string, Book*>& documents();

    const index_map_type& index_map() const;
    
    /**
     * @brief load all books.
     * @return boolean for successful of not
     */
    bool Initialize(bool reload_pages=false); 

    /**
     * @brief parse json of all items. If item exists, change metadata of item, create new book.
     * @param[in] j_items json with all items
     */
    void CreateItemsFromMetadata(nlohmann::json j_Items);

    /**
     * @brief add a book, or rather: add ocr to book
     * @param[in] path path to book.
     * @param[in] key key to book
     */
    void AddBook(std::string path, std::string key, bool reload_pages);

    /**
     * Main search function composing matching conversion, searching for
     * multiple words etc.
     * @param[in] search_object storing searched words (already converted!) and
     * search-options.
     * @return list of all found books as result-object storing additional
     * information.
     */
    std::list<ResultObject> Search(SearchObject& search_object);

    /**
     * Takes search-options and one searched word and calls mathcing
     * search-function (fuzzy/ normal).
     * Takes result-list as refernece, to reduce need of copying.
     * @param[out] results list of results as refernece, to reduce copying time.
     * @param[in] word which was searched.
     * @param[in] search_options 
     */
    void DoSearch(std::map<std::string, ResultObject>& results, std::string word, 
        SearchOptions& search_options);

    void NormalSearch(std::string word, SearchOptions& search_options, 
        std::map<std::string, ResultObject>& results);
    void FuzzySearch(std::string word, SearchOptions& search_options, 
        std::map<std::string, ResultObject>& results);
    bool CheckSearchOptions(SearchOptions& search_options, Book* book);

	  typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;
    typedef std::set<std::pair<std::string, double>, Comp> sorted_set;
    /**
     * @brief sort a map by it's value and return as set.
     * @param[in] unordered_results of books that have been found to contains the searched word
     * @param[in] type of sort algorythem (0: relevance, 1: chronological, 2: alphabetical. 
     * @return list of searchresulst
     */
    sorted_set SortMapByValue(std::map<std::string, double> unordered_results, int type);
    sorted_set SortByRelavance(std::map<std::string, double> unordered_results);
    sorted_set SortChronologically(std::map<std::string, double> unordered_results);
    sorted_set SortAlphabetically(std::map<std::string, double> unordered_results);

    /**
     * Create index map.
     * For each word occuring in each book, in metadata or corpus add word to
     * map. For each word, store all books, in which the word occured, a
     * relevance and a scope. This scope is stored a bit with the different bits
     * indicating where the word has found. The first bit indicates, the word
     * was found in the corpus. All other scopes can are stored in the
     * `reverted_tag_map_` f.e. `reverted_tag_map_["authors"]` returns the bit
     * of the tag "authors".
     */
    void CreateIndexMap();

    void AddWordsFromItem(std::unordered_map<std::string, std::vector<WordInfo>> m, 
        bool corpus, std::string item_key);

    /**
     * Create list words sorted by relevance on the bases of the index map. Pass
     * a scope, to only add words from a particular scope into this map.
     * @param[in, out] list_words to create.
     * @param scope specifying from which scope to take words.
     */
    void CreateListWords(sorted_list_type& listWords, short scope=0);

    /**
     * @brief return a list of 10 words, fitting search Word, sorted by in how many books they apear
     */
    std::list<std::string> GetSuggestions(std::string sWord, std::string sWhere);
    std::list<std::string> GetSuggestions(std::string sWord, sorted_list_type& listWords);

    /**
     * Converts given items to items matching search-config and converts keys to
     * bit representation.
     *
     * 1. All necessary keys are specified in the search-config, so that we can convert the json 
     * for earch item to a map storing f.e. key, title, author... 
     * Example: `{"data":{"title":"[title]"}}` become `{"title":"[title]"}`
     * 2. Convert each key (title, key, author, etc.) specified in config, to a
     * bit representation: f.e. "title" → 1, "key" → 2, "author" → 4 ...
     *
     * This gives us the possibility to store the data in simple maps. By
     * converting the original key to a bit representation, RAM is reduced and
     * it is easy to specify in which tags a word was found.
     * F.e. 5 → found in "title" and "author, or 7 → found in "title", "author", "key".
     *
     * @param[in] metadata_items in original "random" format (f.e. from a zotero, or citavi).
     * @return a list of maps, each storing the bit-representation of a tag its value.
     * F.e. `[{1:"Picture of Dorian Gray", 2:"D7YH2W", 3:"Wilde, Oscar}, {...}, ...]`
     */
    std::vector<std::map<short, std::string>> ConvertMetadata(const nlohmann::json& metadata_items);
}; 

#endif
