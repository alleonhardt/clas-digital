#include <iostream>
#include <string>
#include <list>
#include <map>
#include <fstream>
#include <dirent.h>
#include "CBook.hpp"
#include "CSearch.hpp"

#pragma once 

class CBookManager
{
private:

    //Map of all books
    std::map<std::string, CBook> m_mapBooks;
    std::map<std::string, std::map<std::string, CBook*>> m_mapWords;
    std::map<std::string, std::map<std::string, CBook*>> m_mapWordsTitle;

public:

    // **** getter **** //

    /**
    * @return map of all book
    */
    const std::map<std::string, CBook>& getMapOfBooks();

    /**
    * @brief load all books.
    * @return boolean for successful of not
    */
    bool initialize(); 

    /**
    * @brief update zotero and get json with all books. Replace all metadata. If a new book
    *        was found, call "addBook"
    */
    void updateZotero(nlohmann::json j_Items);

    /**
    * @brief add a book
    */
    void addBook(std::string sKey);

    /**
    * @brief search function calling fitting function from search class
    * @return list of all found books
    */
    std::map<std::string, CBook*>* search(std::string sWord, bool ocr, bool title);

    /**
    * @brief create map of all words (key) and books in which the word occurs (value)
    */
    void createMapWords();

    /**
    * @brief create map of all words (key) and book-titles in which the word occurs (value)
    */
    void createMapWordsTitle();
}; 



