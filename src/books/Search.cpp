#include "CSearch.hpp"

/**
* @brief constructor
*/
CSearch::CSearch(std::string sWord) {
    m_sWord = sWord;
}


/**
* @param[in] mapWords map of all words with a list of books in which this where accures
* @return list of books
*/
void CSearch::normalSearch(std::map<std::string, std::list<CBook*>>& mapWords, 
                                                    std::map<std::string, CBook*>& mapSR) {
    std::list<CBook*> listBooks = mapWords[m_sWord];
    for(auto it=listBooks.begin(); it!=listBooks.end(); it++)
        mapSR[(*it)->getKey()] = (*it);
}

