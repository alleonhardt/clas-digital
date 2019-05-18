#include "CBook.hpp"

CBook::CBook () {}

/**
* @Brief Constructor
* @param[in] sPath Path to book
* @param[in] map map of words in book
*/
CBook::CBook(nlohmann::json jMetadata) : m_Metadata(jMetadata)
{
    m_sKey = jMetadata["key"];
    m_sKey = "";
    m_bOcr = false;
}


// **** GETTER **** //

/**
* @return Key of the book, after extracting it from the path
*/
const std::string& CBook::getKey() { 
    return m_sKey;
}

/**
* @return Path to directory of the book
*/
const std::string& CBook::getPath() {
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
* @return Boolean, whether book contains ocr or not 
*/
bool CBook::getOcr() {
    return m_bOcr;
}

/**
* @return map of all words in book 
*/
const std::map<std::string, int>& CBook::getMapWords() {
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
* @return title of book
*/
std::string CBook::getTitle() {
    return m_Metadata.getTitle();
}

/**
* @return date or -1 if date does not exists or is currupted
*/
int CBook::getDate() {
    return m_Metadata.getDate();
}


// **** SETTER **** //
    
/**
* @param[in] path set Path to book)
*/
void CBook::setPath(std::string sPath) {
    m_sPath = sPath;
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
    std::ifstream readWords(m_sPath + "/words.txt");
    if(!readWords || readWords.peek() == std::ifstream::traits_type::eof() )
    {
        function.createMapOfWords(getOcrPath(), m_Words);
        safeMapOfWords();
    }
    else
        function.loadMapOfWords(m_sPath + "/words.txt", m_Words);

    m_bOcr = true;
}

/**
* @brief safe created word list to file
*/
void CBook::safeMapOfWords()
{
    //Open path to words
    std::string sPathToWords = m_sPath + "/words.txt";
    std::ofstream write(sPathToWords);

    //Iterate over map of words and write all words to file
    for(auto it=m_Words.begin(); it != m_Words.end(); it++)
        write << it->first << "\n";

    write.close();
}

