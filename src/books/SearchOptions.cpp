#include "CSearchOptions.hpp"

/**
* @breif default constructor.
*/
CSearchOptions::CSearchOptions()
{
    m_chSearchedWord="";
    m_secondWord = "";
    m_fuzzyness = 0;
    m_onlyTitle = false;
    m_From = 0;
    m_To = 2018;
    m_onlyOCR = false;
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
CSearchOptions::CSearchOptions(std::string chSearchedWord, int fuzzyness, std::vector<std::string> 
                    sCollections, bool onlyTitle, bool onlyOCR, std::string slastName, int from, int to) 
{
    func::convertToLower(chSearchedWord);
    m_chSearchedWord.assign(chSearchedWord);
    m_secondWord = "";
    m_fuzzyness = fuzzyness;
    m_sCollections = sCollections;
    m_onlyTitle = onlyTitle;
    m_slastName.assign(slastName); 
    m_From = from;
    m_To = to;
    m_onlyOCR = onlyOCR;

}

/**
* @brief initialise search options outside of constructor
* @param[in] chSearchedWord searched word
* @param[in] fuzzyness value of fuzzyness
* @param[in] sCollections collections in which to be searched
* @param[in] onlyTitle search only in title?
* @param[in] onlyOCR search only in ocr (if exists)
* @param[in] slastName las name of author
* @param[in] from date from which books shall be searched
* @param[in] to date to which books shall be searched
**/
void CSearchOptions::initialise(std::string chSearchedWord, int fuzzyness, std::vector<std::string> 
                            pillar, bool onlyTitle, bool onlyOCR,std::string slastName, int from, int to) 
{
    m_chSearchedWord.assign(chSearchedWord);
    m_fuzzyness = static_cast<double>(fuzzyness)/10;
    m_sCollections = pillar;

    m_onlyTitle = onlyTitle;
    m_slastName.assign(slastName); 
    m_From = from;
    m_To = to;
    m_onlyOCR = onlyOCR;
}


//Getter

/**
* @return searched word
**/
std::string CSearchOptions::getSearchedWord() const {
    return m_chSearchedWord;
}

/**
* @return second searched word
**/
std::string CSearchOptions::getSecondWord() const {
    return m_secondWord;
}
/**
* @return selected fuzzyness
**/
int CSearchOptions::getFuzzyness() const {
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
* @param[in] searchedWord new searched word 
*/
void CSearchOptions::setSearchedWord(std::string searchedWord) {
    m_chSearchedWord = searchedWord;
}

/**
* @param[in] secondWord
*/
void CSearchOptions::setSecondWord(std::string secondWord) {
    m_secondWord = secondWord;
}
