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
#include "CBook.hpp"
#include "CSearch.hpp"
#include "CSearchOptions.hpp"
#include "func.hpp"

#pragma once 

class CBookManager
{
private:

    //Map of all books
    std::unordered_map<std::string, CBook*> m_mapBooks;
    std::unordered_map<std::string, std::map<std::string, double>> m_mapWords;
    std::unordered_map<std::string, std::map<std::string, double>> m_mapWordsTitle;

    std::shared_mutex m_searchLock;

public:

    // **** getter **** //

    /**
    * @return map of all book
    */
    std::unordered_map<std::string, CBook*>& getMapOfBooks();

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
    std::list<std::string>* search(CSearchOptions* searchOptions);

    /**
    * @brief convert to list
    * @return list of searchresulst
    */
    std::list<std::string>* convertToList(std::map<std::string, double>* mapResults);

    /**
    * @brief create map of all words (key) and books in which the word occurs (value)
    */
    void createMapWords();

    /**
    * @brief create map of all words (key) and book-titles in which the word occurs (value)
    */
    void createMapWordsTitle();

}; 



