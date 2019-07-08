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
* @return searchOptions
*/
CSearchOptions* CSearch::getSearchOptions() {
    return m_sOpts;
}
    
/**
* @return progress
*/
float CSearch::getProgress() {
    return m_fProgress;
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
std::map<std::string, CBook*>* CSearch::search(std::map<std::string, std::map<std::string, CBook*>>& mWs, std::map<std::string, std::map<std::string, CBook*>>& mWsTitle, std::map<std::string, double>& matches)
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
            containsSearch(mWs, mapSearchresults, matches);
        if(m_sOpts->getOnlyOcr() == false)
            containsSearch(mWsTitle, mapSearchresults, matches);
    }

    //Fuzzy Search
    else
    {
        //Search in ocr and/ or in title
        if(m_sOpts->getOnlyTitle() == false)
            fuzzySearch(mWs, mapSearchresults, matches);
        if(m_sOpts->getOnlyOcr() == false)
            fuzzySearch(mWsTitle, mapSearchresults, matches);
    }

    //Check search-options and remove books from search results, that don't match
    removeBooks(mapSearchresults, matches);

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
                           std::map<std::string, CBook*>* mapSR, std::map<std::string, double>& matches)
{
    unsigned int counter = 0;
	unsigned int counter2 = 0;

    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        if(it->first.find(m_sWord.c_str())!=std::string::npos)
        {
            if(it->first.length() == m_sWord.length())
                myInsert(mapSR, it->second, matches, 0);
            else
                myInsert(mapSR, it->second, matches, 2);
        }

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
                          std::map<std::string, CBook*>* mapSR, std::map<std::string, double>& matches)
{
    unsigned int counter = 0;
	unsigned int counter2 = 0;
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        double value = -1;
        if(func::compare(it->first.c_str(), m_sWord.c_str()) == true)
            myInsert(mapSR, it->second, matches, 0);

        else if (func::contains(it->first.c_str(), m_sWord.c_str()) == true)
        {
            //size_t ld= fuzzy::levenshteinDistance(it->first.c_str(), m_sWord.c_str());
            //double score = static_cast<double>(ld)/ std::max(it->first.length(), m_sWord.length());
            myInsert(mapSR, it->second, matches, 2);
        }

        else if(fuzzy::fuzzy_cmp(it->first.c_str(), m_sWord.c_str(), value) == true)
            myInsert(mapSR, it->second, matches, value);

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
* @brief check whether searched word matches with author of a book.
*/
std::map<std::string, CBook*>* CSearch::checkAuthor(std::map<std::string, CBook>& mapBooks)
{
    std::map<std::string, CBook*>* mapSR = new std::map<std::string, CBook*>;

    int fuzzyness=m_sOpts->getFuzzyness();
    for(auto it=mapBooks.begin(); it!=mapBooks.end(); it++)
    {
        if(it->second.getOcr() == false && m_sOpts->getOnlyOcr() == true)
            continue;

        if(fuzzyness == 0 && func::compare(it->second.getAuthor(), m_sWord) == true)
            mapSR->insert(std::pair<std::string, CBook*>(it->first, &it->second));

        else if(fuzzyness == 1 && it->second.getAuthor().find(m_sWord) != std::string::npos)
            mapSR->insert(std::pair<std::string, CBook*>(it->first, &it->second));

        else if(fuzzyness == 2 && fuzzy::fuzzy_cmp(it->second.getAuthor().c_str(), m_sWord) == true)
            mapSR->insert(std::pair<std::string, CBook*>(it->first, &it->second));
    }

    return mapSR;
}


/**
* @brief remove all books that don't agree with searchOptions.
* @param[in, out] mapSR map of search results
*/
void CSearch::removeBooks(std::map<std::string, CBook*>* mapSR, std::map<std::string, double>& matches)
{
    for(auto it=mapSR->begin(); it!=mapSR->end(); it++)
    {
        if(checkSearchOptions(it->second) == false)
        {
            matches.erase(it->first);
            mapSR->erase(it);
        }
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
}

/*
* @brief inserts searchResults into map of searchresults and assigns value of match
* @param[out] mapSR
* @param[in] found
* @param[out] matches
* @param[in] value
*/
void CSearch::myInsert(std::map<std::string, CBook*>* mapSR, std::map<std::string, CBook*>& found,
                        std::map<std::string, double>& matches, double value)
{
    for(auto it=found.begin(); it!=found.end(); it++)
    {
        mapSR->insert(std::pair<std::string, CBook*>(it->first, it->second));
        if(matches.count(it->first) == 0) 
            matches[it->first] = value;

        else if(matches[it->first] > value)
            matches[it->first] = value;
    }
}

           

/**
* @brief delete searchOptions
*/
void CSearch::deleteSearchOptions() {
    delete m_sOpts;
}
