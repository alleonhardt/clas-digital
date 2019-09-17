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
    m_bOcr = false;

    //Metadata
    m_sAuthor = m_Metadata.getAuthor();
    func::convertToLower(m_sAuthor);
    m_date = m_Metadata.getDate();
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
    return m_sAuthor;
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
    return m_date;
}

/**
* @brief return whether book is publically accessable 
*/
bool CBook::getPublic() {
    if(getDate() == -1 || getDate() > 1919)
        return false;
    else
        return true;
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

    std::ifstream readWords(m_sPath + "/pages.txt");
    if(!readWords || readWords.peek() == std::ifstream::traits_type::eof() )
    {
        alx::cout.write(alx::console::yellow_black, "Creating map of words... \n");
        safePages();
    }

    m_bOcr = true;
}

/**
* @brief safe map of all words and pages on which word occures to disc
*/
void CBook::safePages()
{
    std::map<std::string, std::vector<size_t>> mapWordsPages;

    func::extractPages(getOcrPath(), mapWordsPages);
    alx::cout.write("size: ", mapWordsPages.size(), "\n");

    std::ofstream write(m_sPath + "/pages.txt");
    for(auto it=mapWordsPages.begin(); it!=mapWordsPages.end(); it++)
    {
        std::string sBuffer;
        sBuffer += it->first + ";";
        for(size_t page : it->second) {
            sBuffer += std::to_string(page) + ",";
        }
        sBuffer.pop_back();
        sBuffer += "\n";
        write << sBuffer;
    }
    write.close();
}

/**
* @brief load words and pages on which word occures into map
*/
void CBook::loadPages(std::map<std::string, std::vector<size_t>>& mapWordsPages)
{
    //Load map
    std::ifstream read(m_sPath + "/pages.txt");

    //Check whether map could be loaded
    if(!read)
        return;
   
    std::string sBuffer;
    while(!read.eof())
    {
        //Read new line
        getline(read, sBuffer);

        if(sBuffer.length() < 2)
            continue;

        //Extract word (vec[0] = word, vec[1] = sBuffer
        std::vector<std::string> vec = func::split2(sBuffer, ";");

        //Extract pages and convert to size_t
        std::vector<size_t> pages;
        std::vector<std::string> vecPages = func::split2(vec[1], ",");
        for(auto page : vecPages) 
        {
            if(isdigit(page[0]))
                pages.push_back(std::stoi(page));
        }

        //Add word and pages to map
        mapWordsPages[vec[0]] = pages;
    }
} 

/**
* @brief getPages calls the matching getPages... function according to fuzzyness
*/
std::map<int, std::vector<std::string>>* CBook::getPages(std::string sInput, int fuzzyness)
{
    std::vector<std::string> vWords;
    func::convertToLower(sInput);
    func::split(sInput, "+", vWords);

    if(fuzzyness == 0)
        return getPages(vWords, &CBook::findPagesFull);
    else if(fuzzyness == 1)
        return getPages(vWords, &CBook::findPagesContains);
    else
        return getPages(vWords, &CBook::findPagesFuzzy);
}

std::map<int, std::vector<std::string>>* CBook::getPages(std::vector<std::string>& vWords, std::map<int, std::vector<std::string>>* (CBook::*find)(std::string, std::map<std::string, std::vector<size_t>>&))
{
    //Load map of Words 
    std::map<std::string, std::vector<size_t>> mapWordsPages;
    loadPages(mapWordsPages);

    //Create map of pages and found words for first word
    std::map<int, std::vector<std::string>>* mapPages = (this->*find)(vWords[0], mapWordsPages);

    for(size_t i=1; i<vWords.size(); i++)
    {
        std::map<int, std::vector<std::string>>* mapPages2 = (this->*find)(vWords[i], mapWordsPages);

        //Remove all elements from mapPages, which do not exist in results2. 
        removePages(mapPages, mapPages2);

        delete mapPages2;
    }

    return mapPages;
}


//Create map of pages and found words for i-word (full-search)
std::map<int, std::vector<std::string>>* CBook::findPagesFull(std::string sWord, std::map<std::string, std::vector<size_t>>& mapWordsPages)
{
    //Create empty list of pages
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    for(auto page : mapWordsPages[sWord])
        (*mapPages)[page] = {};

    return mapPages;
}


//Create map of pages and found words for i-word (contains-search)
std::map<int, std::vector<std::string>>* CBook::findPagesContains(std::string sWord, std::map<std::string, std::vector<size_t>>& mapWordsPages)
{
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    for(auto it=mapWordsPages.begin(); it!=mapWordsPages.end(); it++)
    {
        if(it->first.find(sWord) != std::string::npos)
        {
            for(auto page : it->second) {
                if(mapPages->count(page) > 0)
                    mapPages->at(page).push_back(it->first);
                else 
                    mapPages->insert(std::pair<int, std::vector<std::string>> (page, {it->first}));
            }
        }
    }

    return  mapPages;
}

//Create map of pages and found words for i-word (fuzzy-search)
std::map<int, std::vector<std::string>>* CBook::findPagesFuzzy(std::string sWord, std::map<std::string, std::vector<size_t>>& mapWordsPages)
{
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    for(auto it=mapWordsPages.begin(); it!=mapWordsPages.end(); it++)
    {
        if(fuzzy::fuzzy_cmp(it->first, sWord) == true)
        {
            for(auto page : it->second) {
                if(mapPages->count(page) > 0)
                    mapPages->at(page).push_back(it->first);
                else
                    mapPages->insert(std::pair<int, std::vector<std::string>> (page, {it->first}));
            }
        }
    }

    return mapPages;
}       

/**
* @brief Remove all elements from mapPages, which do not exist in results2. 
*  For all other elements, add the found string from results to on this page to the result
* @param[in, out] results1
* @param[in] results2
* @return map of pages and words found on this page
*/
void CBook::removePages(std::map<int, std::vector<std::string>>* results1, std::map<int, std::vector<std::string>>* results2)
{
    for(auto it=results1->begin(); it!=results1->end(); ++it)
    {
        if(results2->count(it->first) == 0)
            results1->erase(it);
        else
            it->second.insert(it->second.end(), (*results2)[it->first].begin(), (*results2)[it->first].end());
    }
}

/**
* @brief Remove all elements from mapPages, which do not exist in results2. 
* @param[in, out] results1
* @param[in] results2
* @return map of pages and words found on this page
*/
void CBook::removePages2(std::map<int, std::vector<std::string>>* r1, std::map<int, std::vector<std::string>>* r2)
{
    for(auto it=r1->begin(); it!=r1->end(); ++it)
    {
        if(r2->count(it->first) == 0 && r2->count(it->first+1) == 0 && r2->count(it->first-1) == 0)
            r1->erase(it);
    }
}

/**
* @brief getNumMatches returns number of matches of this book
*/
int CBook::getNumMatches(std::string sInput, int fuzzyness)
{
    std::vector<std::string> vWords;
    func::convertToLower(sInput);
    func::split(sInput, "+", vWords);

    if(fuzzyness == 0)
        return getNumMatches(vWords, &CBook::findPagesFull);
    else if(fuzzyness == 1)
        return getNumMatches(vWords, &CBook::findPagesContains);
    else
        return getNumMatches(vWords, &CBook::findPagesFuzzy);
}

int CBook::getNumMatches (std::vector<std::string>& vWords, std::map<int, std::vector<std::string>>* (CBook::*find)(std::string, std::map<std::string, std::vector<size_t>>&))
{
    //Load map of Words 
    std::map<std::string, std::vector<size_t>> mapWordsPages;
    loadPages(mapWordsPages);

    //Create map of pages and found words for first word
    std::map<int, std::vector<std::string>>* mapPages = (this->*find)(vWords[0], mapWordsPages);

    for(size_t i=1; i<vWords.size(); i++)
    {
        std::map<int, std::vector<std::string>>* mapPages2 = (this->*find)(vWords[i], mapWordsPages);

        //Remove all elements from mapPages, which do not exist in results2. 
        removePages2(mapPages, mapPages2);

        delete mapPages2;
    }

    return mapPages->size();
}

std::string CBook::getPreview(std::string sWord, int fuzzyness)
{
    //Read ocr
    std::ifstream read(getOcrPath(), std::ios::in);

    //Check, whether ocr could be loaded
    if(!read)
        return "No preview";

    std::string result;
    std::string resultContains;
    std::string resultFuzzy;
    std::string sBuffer;
    while(!read.eof())
    {
        //Read current line
        getline(read, sBuffer);

        //Check whether ". " was found
        if(sBuffer.find(". ") != std::string::npos)
        {
            result.append(sBuffer.substr(0, sBuffer.find(". ")));

            std::map<std::string, int> mapWords;
            func::extractWordsFromString(result, mapWords);

            //Full search
            for(auto it : mapWords) {
                if(it.first.compare(sWord) == 0)
                    return result;
            }

            if(fuzzyness == 0)
            {
                result = sBuffer.substr(sBuffer.find(". ")+2);
                continue;
            }

            //Contains search
            if(result.find(sWord) != std::string::npos)
                resultContains = result;

            if(fuzzyness == 1)
            {
                result = sBuffer.substr(sBuffer.find(". ")+2);
                continue;
            }

            //Contains search
            for(auto it : mapWords) {
                if(fuzzy::fuzzy_cmp(it.first, sWord) == true)
                    resultFuzzy= result;
            }

            result = sBuffer.substr(sBuffer.find(". ")+2);
        }
        else
            result.append(sBuffer);
    }


    if(resultContains != "")
        return resultContains;
    if(resultFuzzy != "")
        return resultFuzzy;

    return "No Preview";
}



