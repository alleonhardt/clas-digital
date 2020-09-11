#include "search_options.h"

/**
* @breif default constructor.
*/
SearchOptions::SearchOptions()
{
    m_chSearchedWord="";
    m_fuzzyness = false;
    m_onlyTitle = false;
    m_From = 0;
    m_To = 2018;
    m_onlyOCR = false;
    m_fullAccess = false;
}

/**
* @brief Constructor
* @param[in] chSearchedWord searched word
* @param[in] fuzzyness value of fuzzyness
* @param[in] sCollections collections in which to be searched
* @param[in] onlyTitle search only in title?
* @param[in] onlyOCR search only in ocr (if exists)
* @param[in] slastName las name of author
* @param[in] from date from which books shall be searched
* @param[in] to date to which books shall be searched
**/
SearchOptions::SearchOptions(std::string chSearchedWord, bool fuzzyness, std::vector<std::string> sCollections, std::string sScope, std::string slastName, int from, int to, bool full, std::string filterResults)
{
    func::convertToLower(chSearchedWord);
    std::replace(chSearchedWord.begin(), chSearchedWord.end(), ' ', '+');
	m_chSearchedWord = func::convertStr(chSearchedWord);
    m_fuzzyness = fuzzyness;
    m_sCollections = sCollections;
	func::convertToLower(slastName);
    m_slastName.assign(slastName); 
    m_From = from;
    m_To = to;

    m_onlyTitle = false;
    m_onlyOCR = false;
    if(sScope=="metadata") m_onlyTitle = true;
    if(sScope=="body")     m_onlyOCR = true;

    m_fullAccess = full;

    m_filterResults = 0;
    if (filterResults == "alphabetically") m_filterResults = 2;
    if (filterResults == "chronologically") m_filterResults = 1;
}


//Getter

/**
* @return searched word
**/
std::string SearchOptions::getSearchedWord() const {
    return m_chSearchedWord;
}

/**
* @return selected fuzzyness
**/
bool SearchOptions::getFuzzyness() const {
    return m_fuzzyness;
}

/**
* @return selected pillars
**/
std::vector<std::string> SearchOptions::getCollections() const {
    return m_sCollections;
}

/**
* @return whether search only in title 
**/
bool SearchOptions::getOnlyTitle() const {
    return m_onlyTitle;
}

/**
* @return whether search only in ocr (if exists)
**/
bool SearchOptions::getOnlyOcr() const {
    return m_onlyOCR;
}
/**
* @return last name of selected author
*/
std::string SearchOptions::getLastName() const {
    return m_slastName;
}

/**
* @return year from which books shall be searched in
**/
int SearchOptions::getFrom() const {
    return m_From;
}

/**
* @return year to which books shall be searched
**/
int SearchOptions::getTo() const {
    return m_To;
}

/**
* @return get user access
*/
bool SearchOptions::getAccess() const {
    return m_fullAccess;
}

/**
* @return return whether results shall be filtered or not
*/
size_t SearchOptions::getFilterResults() const {
    return m_filterResults;
}

/**
* @param[in] searchedWord new searched word 
*/
void SearchOptions::setSearchedWord(std::string searchedWord) {
    m_chSearchedWord = searchedWord;
}


