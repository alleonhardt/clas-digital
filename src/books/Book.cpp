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
    m_sAuthor = func::convertToLower(m_metadata.getAuthor());
    m_date    = m_Metadata.getDate();
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
std::unordered_map<std::string, std::list<std::string>>& CBook::getMapFuzzy() {
    return m_mapFuzzy;
}
std::unordered_map<std::string, int>& getMapRelevance() {
    return m_mapRelevance;
}


/**
* @return lastName, or Name of author
*/
std::string CBook::getAuthor() {
    return m_sAuthor;
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

void CBook::createPages()
{
    std::ifstream read(getOcrPath());

    std::string sBuffer = "";
    pageCounter = 0;

    while(!read.eof()) {

        std::string sLine;
        getline(read, sLine);

        if(func::checkPage(sLine) == true) {
            for(auto it : func::extractWordsFromString(sBuffer)) {
                if(m_mapWordsPages.count(it->first) > 0) {
                    m_mapWordsPages[it->first].push_back(pageCounter);
                    m_mapRelevance[it->first] += it->second;
                }
                else {
                    m_mapWordsPages[it->first] = {pageCounter};
                    m_mapRelevance[it->first] = it->second;
                }
            }
            pageCounter++;

            //Create new page
            std::ofstream write(m_sPath + "/page" + pageCounter + ".txt")
            write << sBuffer;
            write.close();
            sBuffer = "";
        }

        else
            sBuffer += sLine;
    }
    read.close();
    m_numPages = pageCounter;

    safePages();
}
    

/**
* @brief safe map of all words and pages on which word occures to disc
*/
void CBook::safePages()
{
    std::ofstream write(m_sPath + "/pages_new.txt");
    write << m_numPages;
    for(auto it : m_mapWordsPages)
    {
        std::string sBuffer;
        sBuffer += it->first + ";";
        for(size_t page : it->second) {
            sBuffer += std::to_string(page) + ",";
        }
        sBuffer.pop_back();
        sBuffer += ";" + m_mapRelevance[it->first];
        sBuffer += "\n"; 
        write << sBuffer;
    }

    write.close();
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

    //Read number of pages
    getline(read, sBuffer);
    m_numPages = stoi(sBuffer);

    //Read words, pages, and relevance
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
    else
        return getPages(vWords, &CBook::findPagesFuzzy);
}

std::map<int, std::vector<std::string>>* CBook::getPages(std::vector<std::string>& vWords, std::map<int, std::vector<std::string>>* (CBook::*find)(std::string, std::unordered_map<std::string, std::vector<size_t>>&))
{
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


int CBook::getMatches(std::string sInput, int fuzzyness) 
{
    if(fuzzyness == 0)
        return m_mapWordsPages[sInput].size();

    int matches=0;

    if(fuzzyness==2)
    {
        for(std::string sMatch : m_mapFuzzy[sInput])
            matches += m_mapWordsPages[sMatch].size();
    }
    
    return matches;
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
        return "<span style='color:orange;'> Book is not yet scanned, sorry for that.</span>";
    if(sWord.find("+") != std::string::npos)
        return "<span style='color:orange;'> No preview - (Searching for two words)";


    //*** Get page and match ***//
    std::string sMatch;
    size_t page = getBestMatch(func::returnToLower(sWord), sMatch);

    //Check whether page is correct
    if(page == 10000000) {
        alx::cout.write(alx::console::red_black, getMetadata().getShow(), " - NO PAGE FOUND!!!!\n");
        return "<span style='color:orange;'> No preview";
    }
    
    //*** Get Preview ***//
    std::string finalResult = getPreviewMatch(sMatch, page);

    //Check whether preview found
    if(func::compare(finalResult.c_str(), "No Preview") == true)
        return "<span style='color:orange;'> No Preview";


    //*** Shorten preview if needed ***//
    shortenPreview(finalResult, sMatch.length());
     

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
    //Try Full match
    if(m_mapWordsPages.count(sWord) > 0) {
        sMatch = sWord;
        return m_mapWordsPages[sWord].front();
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
    std::ifstream read(m_sPath + "/page" + std::to_string(page+1) + ".txt", std::ios::in);
    
    //Read kontent
    std::stringstream buffer;
    buffer << t.rdbuf();
    sPage = buffer.str();

    //Find match
    size_t pos = sPage.find(sWord);
    func::extractWordsFromString(sPage)[sWord]

    //Return "error" if not found
    if (pos = std::string::npos)
        return "No preview";
    
    //Define markings:
    size_t front = 0, back = sPage.length();
    if(pos-front > 75) front = pos - 75;
    if(back-pos > 75)  back  = pos + 75;
        
    //Return substring
    return sPage.substring(front, back) + "<mark>" + sPage.substring(pos, pos+sWord.length()) 
                + "</mark>" + sPage.substring(pos+sWord.length(), back);
}


void CBook::shortenPreview(size_t pos, std::string& finalResult, size_t len_match)
{
    //Delete invalid chars front
    for(;;)
    {
        if((int)finalResult.front() < 0)
            finalResult.erase(0,1);
        else if((int)finalResult.back() < 0)
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
