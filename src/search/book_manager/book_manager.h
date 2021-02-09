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

#include "book_manager/book.h"
#include "func.hpp"
#include "gramma.h"
#include "search/search.h"
#include "search/search_options.h"


class BookManager {
  private:
    Dict* full_dict_;
    const std::vector<std::string> upload_points_; ///< mount_points for book-locations.

    std::unordered_map<std::string, Book*> map_books_; ///< map of all books.


    // Map of words / map of words in titles
    typedef std::unordered_map<std::string, std::map<std::string, double>> MAPWORDS;
    MAPWORDS map_words_; ///< Map of all words (in text bodies): word -> ocuring books:score.
    MAPWORDS map_words_title_; /// Map of all words in titles: word -> ocuring books:score.
    MAPWORDS map_words_authors_; ///< Map of all authors: name -> ocuring books:score.
    std::map<std::string, std::vector<std::string>> map_unique_authors_;

    typedef std::list<std::pair<std::string, size_t>> sortedList;
    sortedList list_words_; ///< Sorted list of all words by score (for typeahead).
    sortedList list_authors_; ///< Sorted list of all authors by score (for typeahead). 

    std::shared_mutex search_lock_;

  public:
    ///< Constructor
    BookManager(std::vector<std::string> paths_to_books, Dict* dict);

    // **** getter **** //

    std::unordered_map<std::string, Book*>& map_of_books();
    
    MAPWORDS& map_of_authors(); 

    std::map<std::string, std::vector<std::string>>& map_unique_authors();

    /**
     * @brief Generate lists on information about stored books.
     * - currenttly untracked books.
     * - books with bsb-link but without ocr.
     * - books with Tag "gibtEsBeiBSB" but without ocr.
     * - books with tag "gibtEsBeiBSB" but with ocr.
     * - books with tag "BSBDownLoadFertig" but without ocr.
     * - books in collection "Geschichte des Tierwissens" but without ocr.
     */
    void WriteListofBooksWithBSB();

    /**
     * @brief load all books.
     * @return boolean for successful of not
     */
    bool Initialize(bool reload_pages=false); 

    /**
     * @brief parse json of all items. If item exists, change metadata of item, create new book.
     * @param[in] j_items json with all items
     */
    void UpdateZotero(nlohmann::json j_Items);

    /**
     * @brief add a book, or rather: add ocr to book
     * @param[in] path path to book.
     * @param[in] key key to book
     */
    void AddBook(std::string path, std::string key, bool reload_pages);

    /**
     * @brief search function calling fitting function from search class
     * @param[in] searchOPts 
     * @return list of all found books
     */
    std::list<Book*> DoSearch(SearchOptions* searchOptions);

	  typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;
    typedef std::set<std::pair<std::string, double>, Comp> sorted_set;
    /**
     * @brief sort a map by it's value and return as set.
     * @param[in] unordered_results of books that have been found to contains the searched word
     * @param[in] type of sort algorythem (0: relevance, 1: chronological, 2: alphabetical. 
     * @return list of searchresulst
     */
    sorted_set SortMapByValue(std::map<std::string, double>* unordered_results, int type);

    /**
     * @brief create map of all words (key) and books in which the word occurs (value)
     */
    void CreateMapWords();

    /**
     * @brief create map of all words (key) and book-titles in which the word occurs (value)
     */
    void CreateMapWordsTitle();

    /**
     * @brief create map of all words (key) and author names in which the word occurs (value)
     */
    void CreateMapWordsAuthor();

    /**
     * @brief create list of all words and relevance, ordered by relevance
     */
    void CreateListWords(MAPWORDS& mapWords, sortedList& listWords);

    /**
     * @brief return a list of 10 words, fitting search Word, sorted by in how many books they apear
     */
    std::list<std::string> GetSuggestions(std::string sWord, std::string sWhere);
    std::list<std::string> GetSuggestions(std::string sWord, sortedList& listWords);
}; 

#endif
