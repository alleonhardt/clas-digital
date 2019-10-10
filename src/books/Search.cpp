#include "CSearch.hpp"

/**
* @brief constructor
*/
CSearch::CSearch(CSearchOptions* searchOpts, unsigned long long ID, unsigned int numResults=100000) {
    m_sOpts = searchOpts;
    m_ID = ID;
    m_fProgress = 0.0;
    m_numResults = numResults;
}

// *** GETTER *** //

/**
* @return id of search
*/
unsigned long long  CSearch::getID() {
    return m_ID;
}


/**
* @return searchOptions
*/
CSearchOptions* CSearch::getSearchOptions() {
    return m_sOpts;
}
    
/**
* @return progress
*/
void CSearch::getProgress(std::string& status, float& progress) {
    status = m_sStatus;
    progress = m_fProgress;
}

// *** GETTER (from searchoptions) *** //

/**
* @return searched word from searchOptions
*/
std::string CSearch::getSearchedWord() {
    return m_sOpts->getSearchedWord();
}

/** 
* @return fuzzynes
*/
int CSearch::getFuzzyness() {
    return m_sOpts->getFuzzyness();
}

/** 
* @return onlyTitle
*/
int CSearch::getOnlyTitle() {
    return m_sOpts->getOnlyTitle();
}

// *** SETTER *** //

/**
* param[in] searchedWord set searched word
*/
void CSearch::setWord(std::string sWord) {
    m_sWord = sWord;
}


/**
* @brief used to change progress from bookmanager 
*/
void CSearch::setProgress(float progress) {
    m_fProgress = progress;
}

/**
* @brief used to change stus from bookmanager 
*/
void CSearch::setStatus(std::string sStatus) {
    m_sStatus = sStatus;
}

/**
* @brief calls spezific search function, searches, and creates map of  matches. Removes all 
* books that do not match with search options.
*/
std::map<std::string, CBook*>* CSearch::search(std::unordered_map<std::string, std::map<std::string, CBook*>>& mWs, std::unordered_map<std::string, std::map<std::string, CBook*>>& mWsTitle)
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

    //removeBooks(mapSearchresults, matches);

    return mapSearchresults;
}

/**
* @brief search full-match
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::normalSearch(std::unordered_map<std::string, std::map<std::string, CBook*>>& mapWords, 
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
void CSearch::containsSearch(std::unordered_map<std::string, std::map<std::string, CBook*>>& mapWords, 
                           std::map<std::string, CBook*>* mapSR) 
{
    unsigned int counter = 0;
	unsigned int counter2 = 0;

    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(it->first.find(m_sWord.c_str())!=std::string::npos)
            myInsert(mapSR, it->second, it->first);

        //Calculate progress
		if(counter2>=500)
		{
			counter2=0;
        	m_fProgress = static_cast<float>(counter)/static_cast<float>(mapWords.size());
		}
		counter2++;
        counter++;

        if(mapSR->size() == m_numResults)
            break;
    }
}

/**
* @brief search fuzzy 
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::fuzzySearch(std::unordered_map<std::string, std::map<std::string, CBook*>>& mapWords, 
                          std::map<std::string, CBook*>* mapSR)
{
    unsigned int counter = 0;
    unsigned int counter2 = 0;
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        double value = -1;

        //Full-match (if match: value = 0)
        if(func::compare(it->first.c_str(), m_sWord.c_str()) == true)
            myInsert(mapSR, it->second, it->first);

        //Contains-match (if match: value 2)
        else if (func::contains(it->first.c_str(), m_sWord.c_str()) == true)
            myInsert(mapSR, it->second, it->first);

        //Fuzzy-match (if match: value = levensteindistance)
        else if(fuzzy::fuzzy_cmp(it->first.c_str(), m_sWord.c_str(), value) == true)
            myInsert(mapSR, it->second, it->first);

        //Change Progress
	if(counter2>=500)
	{
	    counter2=0;
	    m_fProgress = static_cast<float>(counter)/static_cast<float>(mapWords.size());
	}

		counter2++;
        counter++;
    }
}

/**
* @brief check whether searched word matches with author of a book.
*/
std::map<std::string, CBook*>* CSearch::checkAuthor(std::map<std::string, CBook>& mapBooks)
{
    //Creating empty map of books in which author occures
    std::map<std::string, CBook*>* mapSR = new std::map<std::string, CBook*>;

    //Get Fuzzyness
    int fuzzyness=m_sOpts->getFuzzyness();

    //Iterate over books
    unsigned int counter = 0;
    for(auto it=mapBooks.begin(); it!=mapBooks.end(); it++)
    {
        if(it->second.getOcr() == false && m_sOpts->getOnlyOcr() == true)
            continue;

        //Search author: full-match 
        if(fuzzyness == 0 && func::compare(it->second.getAuthor(), m_sWord) == true)
            mapSR->insert(std::pair<std::string, CBook*>(it->first, &it->second));

        //Search author: contains-match
        else if(fuzzyness == 1 && it->second.getAuthor().find(m_sWord) != std::string::npos)
            mapSR->insert(std::pair<std::string, CBook*>(it->first, &it->second));

        //Search author: fuzzy-match
        else if(fuzzyness == 2 && fuzzy::fuzzy_cmp(it->second.getAuthor().c_str(), m_sWord) == true)
            mapSR->insert(std::pair<std::string, CBook*>(it->first, &it->second));

        //Change progress
        m_fProgress = static_cast<float>(counter)/static_cast<float>(mapBooks.size()); 
        counter++;
    }

    return mapSR;
}


/**
* @brief remove all books that don't agree with searchOptions.
* @param[in, out] mapSR map of search results
*/
void CSearch::removeBooks(std::map<std::string, CBook*>* mapSR)
{
    //Update status
    m_sStatus = "Checking searchoptions: ";

    unsigned int counter = 0;
    for(auto it=mapSR->begin(); it!=mapSR->end();)
    {
        if(checkSearchOptions(it->second) == false)
            mapSR->erase(it++);
        else
            ++it;

        //Change progress
        m_fProgress = static_cast<float>(counter)/static_cast<float>(mapSR->size()); 
        counter++;
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
        if(func::in(collection, book->getCollections()) == true)
            return true;
    }
    return false;
}

/*
* @brief inserts searchResults into map of searchresults and assigns value of match
* @param[out] mapSR
* @param[in] found
* @param[out] matches
* @param[in] value
*/
void CSearch::myInsert(std::map<std::string, CBook*>* mapSR, std::map<std::string, CBook*>& found, std::string sMatch)
{
    for(auto it=found.begin(); it!=found.end(); it++)
    {
        mapSR->insert(std::pair<std::string, CBook*>(it->first, it->second));
        if(m_sOpts->getFuzzyness() == 1)
            it->second->getMapContains()[m_sWord].push_back(sMatch);
        else
            it->second->getMapFuzzy()[m_sWord].push_back(sMatch);
    }
        
}
           

/**
* @brief delete searchOptions
*/
void CSearch::deleteSearchOptions() {
    delete m_sOpts;
}
