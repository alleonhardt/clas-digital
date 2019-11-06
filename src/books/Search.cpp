#include "CSearch.hpp"

/**
* @brief constructor
*/
CSearch::CSearch(CSearchOptions* searchOpts, std::string sWord) {
    m_sOpts = searchOpts;
    m_sWord = m_sOpts->getSearchedWord();
    m_mapSR = new std::map<std::string, double>;
    m_sWord = sWord;
}

CSearch::~CSearch() {
    delete m_mapSR;
}

// *** GETTER *** //


/**
* @return searchOptions
*/
CSearchOptions* CSearch::getSearchOptions() {
    return m_sOpts;
}

std::string CSearch::getSearchedWord() {
    return m_sWord;
}
    

// *** GETTER (from searchoptions) *** //

/** 
* @return fuzzynes
*/
bool CSearch::getFuzzyness() {
    return m_sOpts->getFuzzyness();
}

/** 
* @return onlyTitle
*/
int CSearch::getOnlyTitle() {
    return m_sOpts->getOnlyTitle();
}

/** 
* @return onlyOcr
*/
int CSearch::getOnlyOcr() {
    return m_sOpts->getOnlyOcr();
}
// *** SETTER *** //

/**
* param[in] searchedWord set searched word
*/
void CSearch::setWord(std::string sWord) {
    m_sWord = sWord;
}


/**
* @brief calls spezific search function, searches, and creates map of  matches. Removes all 
* books that do not match with search options.
*/
std::map<std::string, double>* CSearch::search(MAPWORDS& mWs, MAPWORDS& mWsTitle, std::unordered_map<std::string, CBook*>& mapBooks)
{
    //Normal search (full-match)
    if (getFuzzyness() == false)
    {
        //Search in ocr and/ or in title
        if(getOnlyTitle() == false)
            normalSearch(mWs);
        if(getOnlyOcr() == false)
            normalSearch(mWsTitle);
    }

    //Fuzzy Search
    else
    {
        //Search in ocr and/ or in title
        if(getOnlyTitle() == false)
            fuzzySearch(mWs, mapBooks, false);
        if(getOnlyOcr() == false)
            fuzzySearch(mWsTitle, mapBooks, true);
    }

    //Check for author
    checkAuthor(mapBooks);

    //Check search-options and remove books from search results, that don't match
    removeBooks(mapBooks);

    return m_mapSR;
}

/**
* @brief search full-match
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::normalSearch(MAPWORDS& mapWords)
{
    std::cout << "Searching for " << m_sWord << "\n";
    if(mapWords.count(m_sWord) > 0) {
        std::map<std::string, double> searchResults = mapWords.at(m_sWord);
        m_mapSR->insert(searchResults.begin(), searchResults.end());
    }
}


/**
* @brief search fuzzy 
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::fuzzySearch(MAPWORDS& mapWords, std::unordered_map<std::string, CBook*>& mapBooks, bool t)
{
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        double value = fuzzy::fuzzy_cmp(it->first, m_sWord);
        if(value <= 0.2 && t==false) 
            myInsert(it->second, it->first, mapBooks, value);
        else if(value <= 0.2)
            m_mapSR->insert(it->second.begin(), it->second.end());
    }
}

/**
* @brief check whether searched word matches with author of a book.
*/
void CSearch::checkAuthor(std::unordered_map<std::string, CBook*>& mapBooks)
{
    //Iterate over books
    for(auto it=mapBooks.begin(); it!=mapBooks.end(); it++)
    {
        if(it->second->getOcr() == false && m_sOpts->getOnlyOcr() == true)
            continue;

        //Search author: full-match 
        if(getFuzzyness() == false && func::compare(it->second->getAuthor(), m_sWord) == true)
            m_mapSR->insert(std::pair<std::string, double>(it->first, 0));

        //Search author: fuzzy-match
        else if(getFuzzyness() == true && fuzzy::fuzzy_cmp(it->second->getAuthor(), m_sWord) <= 0.2)
            m_mapSR->insert(std::pair<std::string, double>(it->first, 0));
    }
}


/**
* @brief remove all books that don't agree with searchOptions.
* @param[in, out] mapSR map of search results
*/
void CSearch::removeBooks(std::unordered_map<std::string, CBook*>& mapBooks)
{
    for(auto it=m_mapSR->begin(); it!=m_mapSR->end();)
    {
        if(checkSearchOptions(mapBooks[it->first]) == false)
            m_mapSR->erase(it++);
        else
            ++it;
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
        if(book->getAuthor() != m_sOpts->getLastName())
            return false;
    }

    //*** check date ***//
    int date = book->getDate();
    if(date == -1 || date < m_sOpts->getFrom() || date > m_sOpts->getTo())
        return false;
         
    //*** check pillars ***//
    for(auto const &collection : m_sOpts->getCollections()) {
        if(func::in(collection, book->getMetadata().getCollections()) == true)
            return true;
    }
    return false;
}

/*
* @brief inserts searchResults into map of searchresults and assigns value of match
* @param[out] mapSR
* @param[in] found
* @param[out] sMatch
* @param[in] value
*/
void CSearch::myInsert(std::map<std::string, double>& found, std::string sMatch, std::unordered_map<std::string, CBook*>& mapBooks, double value)
{
    for(auto it=found.begin(); it!=found.end(); it++) {
        (*m_mapSR)[it->first] += it->second*(1-value*5);

        //Add match to map
        if(mapBooks[it->first]->getOcr() == false)
            continue;
        if (mapBooks[it->first]->getMapFuzzy()[m_sWord].front().second > value)
            mapBooks[it->first]->getMapFuzzy()[m_sWord].push_front({sMatch, value});
        else
            mapBooks[it->first]->getMapFuzzy()[m_sWord].push_back({sMatch, value});
    }
}
           

/**
* @brief delete searchOptions
*/
void CSearch::deleteSearchOptions() {
    delete m_sOpts;
}


