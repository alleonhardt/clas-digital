#include <iostream>
#include <string>
#include <list>
#include <map>
#include "CBook.hpp"
#include "func.hpp"

#pragma once 

class CSearch
{
private:
    std::string m_sWord;
public:
    
    /**
    * @brief constructor
    */
    CSearch(std::string sWord);

    /**
    * @param[in] mapWords map of all words with a list of books in which this where accures
    * @param[int mapSR map of search results
    * @return list of books
    */
    void normalSearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords,
                                                            std::map<std::string, CBook*>* mapSR);

};
