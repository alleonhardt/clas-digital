#include <iostream>
#include <experimental/filesystem>
#include <string>
#include <list>
#include <map>
#include <set>
#include <functional> 
#include <algorithm> 
#include <fstream>
#include <dirent.h>
#include <shared_mutex>
#include "src/util/debug.hpp"
#include "CBook.hpp"
#include "CSearch.hpp"
#include "CSearchOptions.hpp"
#include "func.hpp"
#include "src/console/console.hpp"

#pragma once 

class CBookManager
{
private:

    //Map of all books
    std::map<std::string, CBook> m_mapBooks;
    std::map<std::string, std::map<std::string, CBook*>> m_mapWords;
    std::map<std::string, std::map<std::string, CBook*>> m_mapWordsTitle;

    std::map<unsigned long long, CSearch*> m_mapSearchs;

    std::shared_mutex m_searchLock;

public:

    // **** getter **** //

    /**
    * @return map of all book
    */
    std::map<std::string, CBook>& getMapOfBooks();

    /**
    * @brief load all books.
    * @return boolean for successful of not
    */
    bool initialize(); 

    /**
    * @brief parse json of all items. If item exists, change metadata of item, create new book.
    * @param[in] j_items json with all items
    */
    void updateZotero(nlohmann::json j_Items);

    /**
    * @brief add a book, or rather: add ocr to book
    * @param[in] sKey key to book
    */
    void addBook(std::string sKey);

    /**
    * @brief search function calling fitting function from search class
    * @return list of all found books
    */
    std::list<CBook*>* search(unsigned long long id);

    /**
    * @brief convert to list
    * @return list of searchresulst
    */
    std::list<CBook*>* convertToList(std::map<std::string, CBook*>* mapBooks, std::map<std::string, double>& matches);

    /**
    * @brief sort list of found books by number of matches in each book
    * @param[in] listSR (list of search results)
    * @param[in] sInput (input string of user)
    * @param[in] fuzzyness (set fuzzyness)
    * @return sorted list of search results
    */
    std::list<CBook*>* sortByMatches(std::list<CBook*>* listSR, std::string sInput, int fuzzyness);

    /**
    * @brief Convert to list, without sortig (only, when normal search is selected) 
    * @param[in] mapBooks map of books that have been found to contains the searched word
    * @return list of searchresults
    */
    std::list<CBook*>* convertToList(std::map<std::string, CBook*>* mapBooks);


    /**
    * @brief create map of all words (key) and books in which the word occurs (value)
    */
    void createMapWords();

    /**
    * @brief create map of all words (key) and book-titles in which the word occurs (value)
    */
    void createMapWordsTitle();

    /**
    * @brief add a new search
    */
    void addSearch(CSearch* search);

    /**
    * @brief get progress of given search
    * @return float indicating progress
    */
    bool getProgress(unsigned long long id, std::string& status, float& progress);

    /**
    * @brief delete given search and erase from map
    */
    void deleteSearch(unsigned long long id);
}; 



