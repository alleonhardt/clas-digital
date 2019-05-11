#include "CBook.hpp"

CBook::CBook () {}

/**
* @Brief Constructor
* @param[in] sPath Path to book
* @param[in] map map of words in book
*/
CBook::CBook(std::string sPath) : m_Metadata (sPath + "/info.json")
{
    m_sPath = sPath;
    m_bOcr = false;

    createMapWords();    
}

/**
* @return Path to directory of the book
*/
std::string CBook::getPath() {
    return m_sPath;
}

/**
* @return Path to the ocr.txt file
*/     
std::string CBook::getOcrPath() {
    std::string sPath = m_sPath;
    sPath.append("/ocr.txt");
    return sPath;
}

/**
* @return Key of the book, after extracting it from the path
*/
std::string CBook::getKey() { 
    std::string sKey = m_sPath;
    sKey.erase(sKey.begin(), sKey.end()-8);
    return sKey;
}

/**
* @return Boolean, whether book contains ocr or not 
*/
bool CBook::getOcr() {
    return m_bOcr;
}

/**
* @return map of all words in book 
*/
std::map<std::string, int>& CBook::getMapWords() {
    return m_Words;
}


/**
* @return info.json of book
*/
CMetadata& CBook::getMetadata() {
    return m_Metadata;
}


// **** Setter **** //

/**
* @param[in] bool indicating whether book has ocr or not
*/
void CBook::setOcr(bool bOcr) {
    m_bOcr = bOcr;
}

/**
* @param[in] sPath Path to direcory
*/
void CBook::setPath(std::string sPath) {
    m_sPath.assign(sPath);
}

/**
* @return vector with all collections this book is in
*/
std::vector<std::string> CBook::getCollections() {
    return m_Metadata.getCollections();
}

/**
* @return lastName, or Name of author
*/
std::string CBook::getAuthor() {
    return m_Metadata.getAuthor();
}

/**
* @return date or -1 if date does not exists or is currupted
*/
int CBook::getDate() {
    return m_Metadata.getDate();
}


// **** Other Functions **** //

/**
* @brief checks whether book already has map of words, if not it create them
*/
void CBook::createMapWords()
{
    std::ifstream readOcr(getOcrPath());

    //If ocr exists create or load list with words
    if(!readOcr)
        return;

    CFunctions function;
    std::ifstream readWords(getOcrPath()+"/words.txt");
    if(!readWords)
        function.createMapOfWords(getOcrPath(), m_Words);
    else
        function.loadMapOfWords(getOcrPath()+"/words.txt", m_Words);

    m_bOcr = true;
}



