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

        //Etract pages and convert to size_t
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

//Remove all elements from mapPages, which do not exist in results2. 
//For all other elements, add the found string from results to on this page to the result
void CBook::removePages(std::map<int, std::vector<std::string>>* mapPages, std::map<int, std::vector<std::string>>* results2)
{
    for(auto it=mapPages->begin(); it!=mapPages->end(); ++it)
    {
        bool found = false;
        for(auto jt=results2->begin(); jt!=results2->end() || found == true; jt++)
        {
            if(it->first == jt->first) {
                (*mapPages)[it->first].insert((*mapPages)[it->first].end(), jt->second.begin(), jt->second.end());
                found = true;
            }
        }
        if(found == false)
            mapPages->erase(it--);
    }
}
