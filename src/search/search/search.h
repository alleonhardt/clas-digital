/**
 * @author fux
*/

#ifndef CLASDIGITAL_SRC_SEARCH_SEARCH_SEARCH_H
#define CLASDIGITAL_SRC_SEARCH_SEARCH_SEARCH_H

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

#include "book_manager/book.h"
#include "func.hpp"
#include "fuzzy.hpp"
#include "gramma.h"
#include "search/search_options.h"

class Search
{
private:
    Dict* dict_;
    std::string searched_word_;
    SearchOptions* search_options_;
    std::map<std::string, double>* search_results_;

    typedef std::unordered_map<std::string, std::map<std::string, double>> map_words;
    typedef std::unordered_map<std::string, Book*> map_books;
    typedef std::unordered_map<std::string, std::set<std::string>> dict;

public:
    
    /**
    * @brief constructor
    */
    Search(SearchOptions* searchOpts, std::string sWord, Dict* dict);
    ~Search();


    /**
    * @brief calls spezific search function, searches, and creates map of  matches. Removes all 
    * books that do not match with search options.
    */
    std::map<std::string, double>* search(map_words& mWs, map_words& mWsTitle, map_words& mWsAuthor,
                std::unordered_map<std::string, Book*>& mapBooks);

    /**
    * @brief search full-match
    * @param[in] mapWords map of all words with a list of books in which this where accures
    * @param[in, out] mapSR map of search results
    */
    void normalSearch(map_words& mapWords);

    /**
    * @brief search fuzzy 
    * @param[in] mapWords map of all words with a list of books in which this word accures
    * @param[in, out] mapSR searchresults
    */
    void fuzzySearch(map_words& mapWords, map_books& mapBooks, bool t);

    /*
    * @brief inserts searchResults into map of searchresults and assigns value of match
    * @param[out] mapSR
    * @param[in] found
    * @param[out] matches
    * @param[in] value
    */
    void myInsert(std::map<std::string, double>& found, std::string sMatch, map_books& mapBooks, double value);

    /*
    * @brief inserts searchResults into map of searchresults and assigns value of match
    * @param[out] mapSR
    * @param[in] found
    * @param[out] sMatch
    * @param[in] value
    */
    void myInsert(std::map<std::string, double>& found, std::string sMatch, map_books& mapBooks);

    /**
    * @brief remove all books that do not match with searchoptions
    * @param[in, out] mapSR map of search results
    */
    void removeBooks(map_books& mapBooks);

    /**
    * @brief check whether book-metadata matches with searchoptions
    * @param[in] book to be checked
    * return Boolean
    */
    bool checkSearchOptions(Book* book);
};

#endif
