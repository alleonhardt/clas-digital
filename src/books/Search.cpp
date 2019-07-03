#include "CSearch.hpp"

/**
* @brief constructor
*/
CSearch::CSearch(CSearchOptions* searchOpts, unsigned long long ID) {
    m_sOpts = searchOpts;
    m_ID = ID;
    m_fProgress = 0.0;
}

// *** GETTER *** //

/**
* @return id of search
*/
unsigned long long  CSearch::getID() {
    return m_ID;
}

/**
* @return searched word from searchOptions
*/
std::string CSearch::getSearchedWord() {
    return m_sOpts->getSearchedWord();
}

/**
* @return progress
*/
float CSearch::getProgress() {
    return m_fProgress;
}

// *** SETTER *** //

/**
* param[in] searchedWord set searched word
*/
void CSearch::setWord(std::string sWord) {
    m_sWord = sWord;
}


std::map<std::string, CBook*>* CSearch::search(std::map<std::string, std::map<std::string, CBook*>>& mWs,
                                        std::map<std::string, std::map<std::string, CBook*>>& mWsTitle)
{
    alx::cout.write("Searching for ", m_sWord, "\n");

    //Create empty map of searchResults
    std::map<std::string, CBook*>* mapSearchresults = new std::map<std::string, CBook*>;

    //Normal search (full-match)
    if (m_sOpts->getFuzzyness() == 0)
    {
        //Search in ocr and/ or in title
        if(m_sOpts->getOnlyTitle() == false)
            normalSearch(mWs, mapSearchresults);
        if(m_sOpts->getOnlyOcr() == false)
            normalSearch(mWsTitle, mapSearchresults);
    }

    //Contains-Search 
    else if (m_sOpts->getFuzzyness() == 1)
    {
        //Search in ocr and/ or in title
        if(m_sOpts->getOnlyTitle() == false)
            containsSearch(mWs, mapSearchresults);
        if(m_sOpts->getOnlyOcr() == false)
            containsSearch(mWsTitle, mapSearchresults);
    }

    //Fuzzy Search
    else
    {
        //Search in ocr and/ or in title
        if(m_sOpts->getOnlyTitle() == false)
            fuzzySearch(mWs, mapSearchresults);
        if(m_sOpts->getOnlyOcr() == false)
            fuzzySearch(mWsTitle, mapSearchresults);
    }

    //Check search-options and remove books from search results, that don't match
    removeBooks(mapSearchresults);

    return mapSearchresults;
}


/**
* @brief search full-match
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::normalSearch(std::map<std::string, std::map<std::string, CBook*>>& mapWords, 
                                                    std::map<std::string, CBook*>* mapSR)
{
    //Set Progress
    m_fProgress = 1.0;

    if(mapWords.count(m_sWord) > 0) {
        std::map<std::string, CBook*> searchResults = mapWords.at(m_sWord);
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
    unsigned int counter = 0;
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(func::contains(it->first, m_sWord)== true)
            mapSR->insert(it->second.begin(), it->second.end());

        //Calculate progress
        m_fProgress = static_cast<float>(counter)/static_cast<float>(mapWords.size());
        counter++;
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
    unsigned int counter = 0;
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(fuzzy::fuzzy_cmp(it->first.c_str(), m_sWord.c_str()) == true)
            mapSR->insert(it->second.begin(), it->second.end());

        //Calculate progress
        m_fProgress = static_cast<float>(counter)/static_cast<float>(mapWords.size());
        counter++;
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
            

