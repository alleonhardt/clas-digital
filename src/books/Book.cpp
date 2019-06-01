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

    std::ifstream readWords(m_sPath + "/words.txt");
    if(!readWords || readWords.peek() == std::ifstream::traits_type::eof() )
    {
        std::cout << "Creating map of words... \n";
        func::extractWords(getOcrPath(), m_Words);
        safeMapOfWords();
    }
    else
        func::loadMapOfWords(m_sPath + "/words.txt", m_Words);

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


/*
* @param[in] sWord searched word
* @return list of pages on which searched word accures
*/
std::list<int>* CBook::getPagesFull(std::string sWord)
{
    //Create empty list of pages
    std::list<int>* listPages = new std::list<int>;

    //Load ocr
    std::ifstream read(getOcrPath(), std::ios::in);

    //Check whether ocr could be loaded
    if(!read)
        return listPages;

    //Find pages on which searched word occures
    func::convertToLower(sWord);
    unsigned int pageNum = 0;
    unsigned int lastPage = 0;
    while(!read.eof())
    {
        //read buffer
        std::string sBuffer;
        getline(read, sBuffer);

        //Check new page
        if(func::checkPage(sBuffer) == true) {
            pageNum++;
            continue;
        }

        //Check wether we're still on a page where a word has already been found
        if(pageNum == lastPage)
            continue;

        //Search for word in buffer
        func::convertToLower(sBuffer);
        size_t found = sBuffer.find(sWord);


        if(found == std::string::npos)
            continue;

        size_t found2 = sBuffer.find(" ", found);

        size_t len = 0;
        if(found2 == std::string::npos)
            len = sWord.length();
        else
            len = found2-found;

        std::string foundWord = sBuffer.substr(found, len);
        func::transform(foundWord);

        //Check whether search result is a full match
        if(func::compare(foundWord, sWord) == false)
            continue;

        //Add page to list of pages and increase page number
        listPages->push_back(pageNum);
        lastPage = pageNum;
    }

    return listPages;
}

/*
* @param[in] sWord searched word
* @return map of pages with vector of words found on this page
*/
std::map<int, std::vector<std::string>>* CBook::getPagesContains(std::string sWord)
{
    //Create empty map
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    //Load ocr
    std::ifstream read(getOcrPath(), std::ios::in);

    //Check whether ocr has been loaded
    if(!read)
        return mapPages;

    func::convertToLower(sWord);
    unsigned int pageNum = 0;
    while(!read.eof())
    {
        std::string sBuffer;
        getline(read, sBuffer);

        //Check new page
        if(func::checkPage(sBuffer) == true) {
            pageNum++;
            continue;
        }

        func::convertToLower(sBuffer);
        size_t found = sBuffer.find(sWord);

        if(found == std::string::npos)
            continue;

        size_t found2 = sBuffer.find(" ", found);
        size_t len = 0;
        if(found2 == std::string::npos)
            len = sWord.length();
        else
            len = found2-found;

        std::string foundWord = sBuffer.substr(found, len);
        func::transform(foundWord);

        if(mapPages->count(pageNum) > 0)
            mapPages->at(pageNum).push_back(foundWord);
        else
            mapPages->insert(std::pair<int, std::vector<std::string>> (pageNum, {foundWord}));
    }

    return mapPages;
}

/*
* @param[in] sWord searched word
* @return map of pages with vector of words found on this page
*/
std::map<int, std::vector<std::string>>* CBook::getPagesFuzzy(std::string sWord)
{
    //Create empty map
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    std::ifstream read(getOcrPath(), std::ios::in);

    if(!read)
        return mapPages;

    unsigned int pageNum = 0;
    while(!read.eof())
    {
        std::string sBuffer;
        getline(read, sBuffer);

        //Check new page
        if(func::checkPage(sBuffer) == true) {
            pageNum++;
            continue;
        }

        std::string cur;
        for(unsigned int i=0; i<sBuffer.length(); i++)
        {
            if(func::isLetter(sBuffer[i]) == false && cur.length() == 0)
                continue;

            if((sBuffer[i] == ' ' || sBuffer[i] == '.') && cur.length() > 0)
            {
                if(fuzzy::fuzzy_cmp(cur, sWord) == true)
                {
                    if(mapPages->count(pageNum) > 0)
                        mapPages->at(pageNum).push_back(cur);
                    else
                        mapPages->insert(std::pair<int, std::vector<std::string>> (pageNum, {cur}));
                }
                cur = "";
            }

            else
                cur += sBuffer[i];
        }

        /*
        std::map<std::string, int> mapWords;
        func::extractWordsFromString(sBuffer, mapWords);

        for(auto it=mapWords.begin(); it!=mapWords.end(); it++)
        {
            if(fuzzy::fuzzy_cmp(it->first, sWord) == true)
            {
                if(mapPages->count(pageNum) > 0)
                    mapPages->at(pageNum).push_back(it->first);
                else
                    mapPages->insert(std::pair<int, std::vector<std::string>> (pageNum, {it->first}));
            }
        }
        */
    }

    return mapPages;
}
         
