#include "CSearch.hpp"

/**
* @brief constructor
*/
CSearch::CSearch(std::string sWord) {
    func::convertToLower(sWord);
    m_sWord = sWord;
}


/**
* @param[in] mapWords map of all words with a list of books in which this where accures
* @return list of books
*/
void CSearch::normalSearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords, 
                                                    std::map<std::string, CBook*>* mapSR) {

    if(mapWords.count(m_sWord) > 0) {
        std::map<std::string, CBook*> searchResults = mapWords.at(m_sWord);
        mapSR->insert(searchResults.begin(), searchResults.end());
    }
}

