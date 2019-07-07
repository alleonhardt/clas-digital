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
 

/*
* @param[in] sWord searched word
* @return list of pages on which searched word accures
*/
std::list<size_t>* CBook::getPagesFull(std::string sInput)
{
    std::vector<std::string> vWords;
    func::convertToLower(sInput);
    func::split(sInput, "+", vWords);

    //Create empty list of pages
    std::list<size_t>* listPages = new std::list<size_t>;

    //Load map of Words 
    std::map<std::string, std::vector<size_t>> mapWordsPages;
    loadPages(mapWordsPages);

    for(auto page : mapWordsPages[vWords[0]])
        listPages->push_back(page);

    for(size_t i=1; i<vWords.size(); i++)
    {
        std::vector<size_t> results2 = mapWordsPages[vWords[i]];
        for(auto it=listPages->begin(); it!=listPages->end();++i) 
        {
            if(func::in<size_t>((*it), results2) == false)
                listPages->erase(it--);
            else
                it++;
        }
    }

    return listPages;
}

/**
* @brief getPages calls the matching getPages... function according to fuzzyness
*/
std::map<int, std::vector<std::string>>* CBook::getPages(std::string sInput, int fuzzyness)
{
    if(fuzzyness == 0)
        return getPagesFull2(sInput);
    else if(fuzzyness == 1)
        return getPagesContains(sInput);
    else
        return getPagesFuzzy(sInput);
}

/*
* @param[in] sWord searched word
* @return list of pages on which searched word accures
*/
std::map<int, std::vector<std::string>>* CBook::getPagesFull2(std::string sInput)
{
    std::vector<std::string> vWords;
    func::convertToLower(sInput);
    func::split(sInput, "+", vWords);

    //Create empty list of pages
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    //Load map of Words 
    std::map<std::string, std::vector<size_t>> mapWordsPages;
    loadPages(mapWordsPages);

    for(auto page : mapWordsPages[vWords[0]])
        (*mapPages)[page] = {};

    for(size_t i=1; i<vWords.size(); i++)
    {
        std::map<int, std::vector<std::string>>* mapPages2 = new std::map<int, std::vector<std::string>>;
        for(auto page : mapWordsPages[vWords[i]])
            (*mapPages2)[page] = {};

        //Remove all elements from mapPages, which do not exist in results2. 
        removePages(mapPages, mapPages2);
        delete mapPages2;
    }

    return mapPages;
}

/*
* @param[in] sWord searched word
* @return map of pages with vector of words found on this page
*/
std::map<int, std::vector<std::string>>* CBook::getPagesContains(std::string sInput)
{
    std::vector<std::string> vWords;
    func::convertToLower(sInput);
    func::split(sInput, "+", vWords);

    //Load map of words
    std::map<std::string, std::vector<size_t>> mapWordsPages;
    loadPages(mapWordsPages);

    //Create map of pages and found words for first word
    std::map<int, std::vector<std::string>>* mapPages = findPagesContains(vWords[0], mapWordsPages);

    for(unsigned int i=1; i<vWords.size(); i++)
    {
        //Create map of pages and found words for i-word
        std::map<int, std::vector<std::string>>* results2 = findPagesContains(vWords[i], mapWordsPages);

        //Remove all elements from mapPages, which do not exist in results2. 
        //For all other elements, add the found string from results to on this page to the result
        removePages(mapPages, results2); 

        delete results2;
    }

    return mapPages;
}

/*
* @param[in] sWord searched word
* @return map of pages with vector of words found on this page
*/
std::map<int, std::vector<std::string>>* CBook::getPagesFuzzy(std::string sInput)
{
    std::vector<std::string> vWords;
    func::convertToLower(sInput);
    func::split(sInput, "+", vWords);

    //Load map of words
    std::map<std::string, std::vector<size_t>> mapWordsPages;
    loadPages(mapWordsPages);

    //Map of pages + found words for first word
    std::map<int, std::vector<std::string>>* mapPages = findPagesFuzzy(vWords[0], mapWordsPages);

    for(unsigned int i=1; i<vWords.size(); i++)
    {
        //Create map of pages and found words for i-word
        std::map<int, std::vector<std::string>>* results2 = findPagesFuzzy(vWords[i], mapWordsPages);

        //Remove all elements from mapPages, which do not exist in results2. 
        //For all other elements, add the found string from results to on this page to the result
        removePages(mapPages, results2); 

        delete results2;
    }
    return mapPages;
}

//Create map of pages and found words for i-word
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

//Create map of pages and found words for i-word (fuzzy)
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
