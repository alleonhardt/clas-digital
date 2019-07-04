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
	unsigned int counter2 = 0;
	func::convertToLower(m_sWord);

    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(it->first.find(m_sWord.c_str())!=std::string::npos)
            mapSR->insert(it->second.begin(), it->second.end());

        //Calculate progress
		if(counter2>=10000)
		{
			counter2=0;
        	m_fProgress = static_cast<float>(counter)/static_cast<float>(mapWords.size());
		}
		counter2++;
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
	unsigned int counter2 = 0;
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(fuzzy::fuzzy_cmp(it->first.c_str(), m_sWord.c_str()) == true)
            mapSR->insert(it->second.begin(), it->second.end());

		if(counter2>=10000)
		{
			counter2=0;
        	m_fProgress = static_cast<float>(counter)/static_cast<float>(mapWords.size());
		}
		counter2++;
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
    bool match = false;
    for(auto const &collection : m_sOpts->getCollections()) {
        if(func::in(collection, book->getCollections()) == true)
            match = true;
    }
    return match;

    //*** Check correct access rights ***//
    if((book->getDate()==-1 || book->getDate() > 1919) && m_sOpts->getAccess()==false)
        return false;

    return true;
}
            
/**
* @brief convert to list
* @return list of searchresulst
*/
std::list<CBook*>* CSearch::convertToList(std::map<std::string, CBook*>* mapBooks)
{
    std::list<CBook*>* listBooks = new std::list<CBook*>;
    for(auto it=mapBooks->begin(); it!=mapBooks->end(); it++)
        listBooks->push_back(it->second);
    return listBooks;
}


/**
* @brief delete searchOptions
*/
void CSearch::deleteSearchOptions() {
    delete m_sOpts;
}
