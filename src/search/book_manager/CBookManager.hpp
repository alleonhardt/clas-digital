/**
 * @author fux
*/

#ifndef CLASDIGITAL_SRC_SEARCH_BOOKMANAGER_H_
#define CLASDIGITAL_SRC_SEARCH_BOOKMANAGER_H_

#include <algorithm> 
#include <dirent.h>
#include <experimental/filesystem>
#include <functional> 
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <shared_mutex>
#include <string>

#include "book_manager/CBook.hpp"
#include "func.hpp"
#include "search/CSearch.hpp"
#include "search/CSearchOptions.hpp"


class CBookManager
{
private:

    //Map of all books
    std::unordered_map<std::string, CBook*> m_mapBooks;


    //Map of words / map of words in titles
    typedef std::unordered_map<std::string, std::map<std::string, double>> MAPWORDS;
    MAPWORDS m_mapWords;
    MAPWORDS m_mapWordsTitle;
    MAPWORDS m_mapWordsAuthors;
    std::map<std::string, std::vector<std::string>> m_mapUniqueAuthors;

    typedef std::unordered_map<std::string, std::set<std::string>> dict;
    dict m_dict;

    typedef std::list<std::pair<std::string, size_t>> sortedList;
    sortedList m_listWords;
    sortedList m_listAuthors;

    std::shared_mutex m_searchLock;

public:

    //Constructor
    CBookManager();

    // **** getter **** //

    /**
    * @return map of all book
    */
    std::unordered_map<std::string, CBook*>& getMapOfBooks();
    
    /**
    * @return unordered dicionary of authors
    */
    MAPWORDS& getMapofAuthors(); 

    /**
    * @return map of unique authors ([lastName]-[firstNam])
    */
    std::map<std::string, std::vector<std::string>>& getMapofUniqueAuthors();

    void writeListofBooksWithBSB();

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
    std::list<std::string>* convertToList(std::map<std::string, double>* mapResults, int sorting);

    /**
    * @brief create map of all words (key) and books in which the word occurs (value)
    */
    void createMapWords();

    /**
    * @brief create map of all words (key) and book-titles in which the word occurs (value)
    */
    void createMapWordsTitle();

    /**
    * @brief create map of all words (key) and author names in which the word occurs (value)
    */
    void createMapWordsAuthor();

    void createListWords(MAPWORDS& mapWords, sortedList& listWords);

    /**
    * @brief return a list of 10 words, fitting search Word, sorted by in how many books they apear
    */
    std::list<std::string>* getSuggestions(std::string sWord, std::string sWhere);
    std::list<std::string>* getSuggestions(std::string sWord, sortedList& listWords);
}; 

#endif

