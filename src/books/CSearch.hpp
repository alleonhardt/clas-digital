#include <iostream>
#include <string>
#include <list>
#include <map>
#include "CBook.hpp"
#include "CSearchOptions.hpp"
#include "func.hpp"
#include "fuzzy.hpp"

#pragma once 

class CSearch
{
private:
    CSearchOptions* m_sOpts;

public:
    
    /**
    * @brief constructor
    */
    CSearch(CSearchOptions* searchOpts);

    /**
    * @brief search full-match
    * @param[in] mapWords map of all words with a list of books in which this where accures
    * @param[in, out] mapSR map of search results
    */
    void normalSearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords,
                                                            std::map<std::string, CBook*>* mapSR);
    /**
    * @brief search contains
    * @param[in] mapWords map of all words with a list of books in which this where accures
    * @param[in, out] mapSR map of search results
    */
    void containsSearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords,
                                                            std::map<std::string, CBook*>* mapSR);

    /**
    * @brief search fuzzy 
    * @param[in] mapWords map of all words with a list of books in which this word accures
    * @param[in, out] mapSR searchresults
    */
    void fuzzySearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords, 
                                                        std::map<std::string, CBook*>* mapSR);

    /**
    * @param[in, out] mapSR map of search results
    */
    void removeBooks(std::map<std::string, CBook*>* mapSR);

    /**
    * @param[in] book to be checked
    * return Boolean
    */
    bool checkSearchOptions(CBook* book);
};
