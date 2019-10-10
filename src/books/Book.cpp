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

std::unordered_map<std::string, std::vector<size_t>>& CBook::getMapWordsPages() {
    return m_mapWordsPages;
}
std::unordered_map<std::string, std::list<std::string>>& CBook::getMapContains() {
    return m_mapContains;
}
std::unordered_map<std::string, std::list<std::string>>& CBook::getMapFuzzy() {
    return m_mapFuzzy;
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

    loadPages();
    m_bOcr = true;
}

/**
* @brief safe map of all words and pages on which word occures to disc
*/
void CBook::safePages()
{
    std::unordered_map<std::string, std::vector<size_t>> mapWordsPages;

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
void CBook::loadPages(std::unordered_map<std::string, std::vector<size_t>>& mapWordsPages)
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
* @brief load words and pages on which word occures into map
*/
void CBook::loadPages()
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
        m_mapWordsPages[vec[0]] = pages;
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

std::map<int, std::vector<std::string>>* CBook::getPages(std::vector<std::string>& vWords, std::map<int, std::vector<std::string>>* (CBook::*find)(std::string, std::unordered_map<std::string, std::vector<size_t>>&))
{
    //Load map of Words 
    //std::unordered_map<std::string, std::vector<size_t>> mapWordsPages;
    //loadPages(mapWordsPages);

    //Create map of pages and found words for first word
    std::map<int, std::vector<std::string>>* mapPages = (this->*find)(vWords[0], m_mapWordsPages);

    for(size_t i=1; i<vWords.size(); i++)
    {
        std::map<int, std::vector<std::string>>* mapPages2 = (this->*find)(vWords[i], m_mapWordsPages);

        //Remove all elements from mapPages, which do not exist in results2. 
        removePages(mapPages, mapPages2);

        delete mapPages2;
    }

    return mapPages;
}


//Create map of pages and found words for i-word (full-search)
std::map<int, std::vector<std::string>>* CBook::findPagesFull(std::string sWord, std::unordered_map<std::string, std::vector<size_t>>& mapWordsPages)
{
    //Create empty list of pages
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    for(auto page : mapWordsPages[sWord])
        (*mapPages)[page] = {};

    return mapPages;
}


//Create map of pages and found words for i-word (contains-search)
std::map<int, std::vector<std::string>>* CBook::findPagesContains(std::string sWord, std::unordered_map<std::string, std::vector<size_t>>& mapWordsPages)
{
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    for(std::string sMatch : m_mapContains[sWord]) {
        for(auto page : mapWordsPages[sMatch]) 
            (*mapPages)[page] = {sMatch};
    }

    return  mapPages;
}

//Create map of pages and found words for i-word (fuzzy-search)
std::map<int, std::vector<std::string>>* CBook::findPagesFuzzy(std::string sWord, std::unordered_map<std::string, std::vector<size_t>>& mapWordsPages)
{
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    for(std::string sMatch : m_mapFuzzy[sWord]) {
        for(auto page : mapWordsPages[sMatch]) 
            (*mapPages)[page] = {sMatch};
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

int CBook::getMatches(std::string sInput, int fuzzyness) 
{
    //Load map of Words 
    //std::unordered_map<std::string, std::vector<size_t>> mapWordsPages;
    //loadPages(mapWordsPages);


    if(fuzzyness == 0)
        return m_mapWordsPages[sInput].size();

    int matches=0;
    if(fuzzyness==1)
    {
        for(std::string sMatch : m_mapContains[sInput])
            matches += m_mapWordsPages[sMatch].size();
    }

    if(fuzzyness==2)
    {
        for(std::string sMatch : m_mapFuzzy[sInput])
            matches += m_mapWordsPages[sMatch].size();
    }
    
    return matches;
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

int CBook::getNumMatches (std::vector<std::string>& vWords, std::map<int, std::vector<std::string>>* (CBook::*find)(std::string, std::unordered_map<std::string, std::vector<size_t>>&))
{
    //Load map of Words 
    //std::unordered_map<std::string, std::vector<size_t>> mapWordsPages;
    //loadPages(mapWordsPages);

    //Create map of pages and found words for first word
    std::map<int, std::vector<std::string>>* mapPages = (this->*find)(vWords[0], m_mapWordsPages);

    for(size_t i=1; i<vWords.size(); i++)
    {
        std::map<int, std::vector<std::string>>* mapPages2 = (this->*find)(vWords[i], m_mapWordsPages);

        //Remove all elements from mapPages, which do not exist in results2. 
        removePages2(mapPages, mapPages2);

        delete mapPages2;
    }

    return mapPages->size();
}

/**
* @brief get a preview of the page where the searched word has been found
* @param sWord (searched word)
* @param fuzzyness
* @return Preview
*/
std::string CBook::getPreview(std::string sWord)
{
    //*** Read ocr ***//
    std::ifstream read(getOcrPath(), std::ios::in);

    //Check, whether ocr could be loaded, or search is correct
    if(!read)
        return "<span style='color:red;'> NO SCANS EXIST - sorry for that.</span>";
    if(sWord.find("+") != std::string::npos)
        return "No preview - (Searching for two words)";


    //*** Get page and match ***//
    std::string sMatch;
    size_t page = getBestMatch(func::returnToLower(sWord), sMatch);

    //Check whether page is correct
    if(page == 10000000)
    {
        alx::cout.write(alx::console::red_black, getMetadata().getShow(), " - NO PAGE FOUND!!!!\n");
        return "No preview";
    }
    
    //*** Get Preview ***//
    std::string finalResult = getPreviewMatch(sMatch, page);

    //Check whether preview found
    if(func::compare(finalResult.c_str(), "No Preview") == true)
        return "No Preview";


    //*** Highlight found word ***//
    size_t pos = func::returnToLower(finalResult).find(sMatch);
    finalResult.insert(pos, "<mark>");
    finalResult.insert(pos+6+sMatch.length(), "</mark>");


    //*** Shorten preview if needed ***//
    if(finalResult.length() > 150)
        shortenPreview(pos, finalResult, sMatch.length());
     

    //*** Append [...] front and back ***//
    finalResult.insert(0, "[...] ");
    finalResult.append(" [...]");

    return finalResult;
}


/**
* @brief find page with best match. Deliver page and match
* @param[in] sWord (searched word)
* @param[in] fuzzyness
* @param[in, out] sMatch (found match)
* @return Page on which the match was found.
*/
size_t CBook::getBestMatch(std::string sWord, std::string& sMatch)
{
    //Load map of Words
    //std::unordered_map<std::string, std::vector<size_t>> mapWordsPages;
    //loadPages(mapWordsPages);

    //Try Full match
    if(m_mapWordsPages.count(sWord) > 0) {
        sMatch = sWord;
        return m_mapWordsPages[sWord].front();
    }

    //Try Contains match
    if(m_mapContains.count(sWord)>0 && m_mapWordsPages.count(m_mapContains[sWord].front()) > 0) {
        sMatch = m_mapContains[sWord].front();
        return m_mapWordsPages[sMatch].front();
    }

    //Try Fuzzy match
    if(m_mapWordsPages.count(m_mapFuzzy[sWord].front()) > 0) {
        sMatch = m_mapFuzzy[sWord].front();
        return m_mapWordsPages[sMatch].front();
    }

    return 10000000;
} 

/**
* @brief Find preview with matched word (best match), and page on which the match was found.
* @param[in] sWord (best Match)
* @param[in] page (page on which match was found)
* @return preview for this book
*/
std::string CBook::getPreviewMatch(std::string sWord, size_t page)
{
    //Read ocr
    std::ifstream read(getOcrPath(), std::ios::in);

    size_t curPage = 0;
    bool pageFound = false;
    std::string result;
    std::string sBuffer;
    while(!read.eof())
    {
        //Read current line
        getline(read, sBuffer);


        //*** FIND PAGE ***//
        if(pageFound == false)
        {
            if(func::checkPage(sBuffer) == true)
                curPage++;
            if(curPage != page)
                continue;
            if(curPage == page)
                pageFound = true;
        }


        //*** FIND MATCH ***//

        //Check whether ". " was found
        if(sBuffer.find(". ") != std::string::npos)
        {
            result.append(sBuffer.substr(0, sBuffer.find(". ")));

            //create map of words
            std::map<std::string, int> mapWords;
            func::extractWordsFromString(result, mapWords);

            //Full search, return current result if found
            for(auto it : mapWords) {
                if(it.first.find(sWord) != std::string::npos)
                    return result;
            }

            //Create new result with words after "."
            result = sBuffer.substr(sBuffer.find(". ")+2);
        }

        //If no "." found, add complete buffer to result
        else
            result.append(sBuffer);


        //*** FIND END OF PAGE ***//
        if(func::checkPage(sBuffer) == true)
           curPage++;
        if(curPage>(page+2))
            break;
    }

    //Error output, why the match wasn't found (page not found or match not found on page)
    if(pageFound == false)
        alx::cout.write(alx::console::red_black, "PAGE NOT FOUND!\n");
    else
        alx::cout.write(alx::console::red_black, "MATCH NOT FOUDN!\n");

    return "No Preview";
}


void CBook::shortenPreview(size_t pos, std::string& finalResult, size_t len_match)
{
    size_t minus = finalResult.length() - 150;
    double fakFront = static_cast<double>(pos)/static_cast<double>(finalResult.length());

    if(fakFront > 1.0)
        fakFront = 1.0;

    size_t eraseFront = minus*fakFront;
    size_t eraseBack = minus - eraseFront;

    if(eraseFront>6)
        eraseFront-=6;

    finalResult.erase(0, eraseFront);
    finalResult.erase(finalResult.end()-(eraseBack-(len_match+7)), finalResult.end());

    //Delete invalid chars front
    for(;;)
    {
        if((int)finalResult.front() < 0)
            finalResult.erase(0,1);
        else
            break;
    }

    //Delete invalid chars back
    for(;;)
    {
        if((int)finalResult.back() < 0)
            finalResult.pop_back();
        else
            break;
    }
    
    //Check vor invalid literals and escape
    for(unsigned int i=0; i<finalResult.length(); i++)
    {
        if(finalResult[i] == '\"' || finalResult[i] == '\'' || finalResult[i] == '\\') {
            finalResult.insert(i, "\\");
            i++;
        }
    }
}
