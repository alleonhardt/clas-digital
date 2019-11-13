#include "CSearchOptions.hpp"

/**
* @breif default constructor.
*/
CSearchOptions::CSearchOptions()
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
CSearchOptions::CSearchOptions(std::string chSearchedWord, bool fuzzyness, std::vector<std::string> sCollections, bool onlyTitle, bool onlyOCR, std::string slastName, int from, int to, bool full, std::string filterResults)
{
    func::convertToLower(chSearchedWord);
    std::replace(chSearchedWord.begin(), chSearchedWord.end(), ' ', '+');
	m_chSearchedWord = func::convertStr(chSearchedWord);
    m_fuzzyness = fuzzyness;
    m_sCollections = sCollections;
    m_onlyTitle = onlyTitle;
	func::convertToLower(slastName);
    m_slastName.assign(slastName); 
    m_From = from;
    m_To = to;
    m_onlyOCR = onlyOCR;
    m_fullAccess = full;

    m_filterResults = 0;
    if (filterResults == "alphabetically") m_filterResults = 2;
    if (filterResults == "chronologically") m_filterResults = 1;
}


//Getter

/**
* @return searched word
**/
std::string CSearchOptions::getSearchedWord() const {
    return m_chSearchedWord;
}

/**
* @return selected fuzzyness
**/
bool CSearchOptions::getFuzzyness() const {
    return m_fuzzyness;
}

/**
* @return selected pillars
**/
std::vector<std::string> CSearchOptions::getCollections() const {
    return m_sCollections;
}

/**
* @return whether search only in title 
**/
bool CSearchOptions::getOnlyTitle() const {
    return m_onlyTitle;
}

/**
* @return whether search only in ocr (if exists)
**/
bool CSearchOptions::getOnlyOcr() const {
    return m_onlyOCR;
}
/**
* @return last name of selected author
*/
std::string CSearchOptions::getLastName() const {
    return m_slastName;
}

/**
* @return year from which books shall be searched in
**/
int CSearchOptions::getFrom() const {
    return m_From;
}

/**
* @return year to which books shall be searched
**/
int CSearchOptions::getTo() const {
    return m_To;
}

/**
* @return get user access
*/
bool CSearchOptions::getAccess() const {
    return m_fullAccess;
}

/**
* @return return whether results shall be filtered or not
*/
size_t CSearchOptions::getFilterResults() const {
    return m_filterResults;
}

/**
* @param[in] searchedWord new searched word 
*/
void CSearchOptions::setSearchedWord(std::string searchedWord) {
    m_chSearchedWord = searchedWord;
}


