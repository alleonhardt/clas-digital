#include "CSearch.hpp"

/**
* @brief constructor
*/
CSearch::CSearch(CSearchOptions* searchOpts) {
    m_sOpts = searchOpts;
}


/**
* @brief search full-match
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::normalSearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords, 
                                                    std::map<std::string, CBook*>* mapSR)
{
    if(mapWords.count(m_sOpts->getSearchedWord()) > 0) {
        std::map<std::string, CBook*> searchResults = mapWords.at(m_sOpts->getSearchedWord());
        mapSR->insert(searchResults.begin(), searchResults.end());
    }
}

/**
* @brief search contains
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::containsSearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords, 
                                                    std::map<std::string, CBook*>* mapSR) 
{
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(func::contains(it->first, m_sOpts->getSearchedWord()) == true)
            mapSR->insert(it->second.begin(), it->second.end());
    }
}

/**
* @brief search fuzzy 
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::fuzzySearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords, 
                                                    std::map<std::string, CBook*>* mapSR) 
{
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(fuzzy::fuzzy_cmp(it->first.c_str(), m_sOpts->getSearchedWord().c_str()) == true)
            mapSR->insert(it->second.begin(), it->second.end());
    }
}

/**
* @param[in, out] mapSR map of search results
*/
void CSearch::removeBooks(std::map<std::string, CBook*>* mapSR)
{
    for(auto it=mapSR->begin(); it!=mapSR->end(); it++)
    {
        if(checkSearchOptions(it->second) == false)
            mapSR->erase(it);
    }

}

/**
* @param[in] book to be checked
* return Boolean
*/
bool CSearch::checkSearchOptions(CBook* book)
{
    //*** check author ***//
    if(m_sOpts->getLastName().length() > 0)
    {
        if(func::compare(book->getAuthor(), m_sOpts->getLastName()) == false)
            return false;
    }

    //*** check date ***//
    if(book->getDate()==-1 || book->getDate()<m_sOpts->getFrom() || book->getDate()>m_sOpts->getTo())
        return false;
         
    //*** check pillars ***//
    /*
    bool match = false;
    for(auto const &collection : m_sOpts->getCollections()) {
        if(func::in(collection, book->getCollections()) == true)
            return match;
    }*/

    return true;
}
            

