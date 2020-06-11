#include "CBook.hpp"
#include <ctime>

namespace fs = std::filesystem;

CBook::CBook () {}

/**
* @Brief Constructor
* @param[in] sPath Path to book
* @param[in] map map of words in book
*/
CBook::CBook(nlohmann::json jMetadata) : m_metadata(jMetadata)
{
    m_sKey = jMetadata["key"];
    m_sPath = "web/books/"+m_sKey;
    m_hasOcr = false;
    m_hasImages = false;

    //Metadata
    m_sAuthor = m_metadata.getAuthor();
    func::convertToLower(m_sAuthor);
    if(m_sAuthor.size() == 0) m_sAuthor = "no author";
    m_date    = m_metadata.getDate();
}

// **** GETTER **** //
const std::string& CBook::getKey() { return m_sKey;}
const std::string& CBook::getPath() {return m_sPath;}

bool CBook::hasOcr() const { return m_hasOcr;}
bool CBook::hasImages() const { return m_hasImages; }
bool CBook::hasContent() const { return m_hasImages || m_hasOcr; }

std::string CBook::getOcrPath() {
    std::string sPath = m_sPath;
    sPath.append("/ocr.txt");
    return sPath;
}

std::string CBook::getShow() {

    std::string str = m_metadata.getShow();
    if(m_hasOcr == true)
        return str;
    str+="<span style='color:orange;font-size:80%'> Book is not yet scanned, sorry for that.</span>";
    return str;
}

int CBook::getNumPages() { return m_numPages; }
CMetadata& CBook::getMetadata() { return m_metadata; } 

std::unordered_map<std::string, std::tuple<std::vector<size_t>,int,size_t>>& CBook::getMapWordsPages() { 
    return m_mapWordsPages;
}
std::unordered_map<std::string, std::list<std::pair<std::string, double>>>& CBook::getMapFuzzy() {
    return m_mapFuzzy;
}
std::unordered_map<std::string, std::list<std::string>>& CBook::getMapFull() {
    return m_mapFull;
}

std::string CBook::getAuthor() { return m_sAuthor; }
int CBook::getDate() { return m_date; }

bool CBook::getPublic() {
    std::time_t ttime = time(0);
    tm *local_time = localtime(&ttime);

    //Local time number of seconds elapsed since 1. January 1900. 
    if(getDate() == -1 || getDate() >= (local_time->tm_year+1800))
        return false;
    else
        return true;
}
     
// **** SETTER **** //
void CBook::setPath(std::string sPath) { m_sPath = sPath; }


// **** CREATE BOOK AND MAPS (PAGES, RELEVANCE, PREVIEW) **** // 

/**
* @brief checks whether book already has map of words, if not it create them
*/
void CBook::createBook(std::string sPath)
{
    std::ifstream readOcr(getOcrPath());
    for(auto& p: std::filesystem::directory_iterator(sPath))
    {
	    (void)p;
        if(p.path().extension() == ".jpg" || p.path().extension() == ".bmp")
            m_hasImages = true; 
    }

    //If ocr exists create or load list with words
    if(!readOcr)
        return;

    std::ofstream writeJson(m_sPath + "/info.json");
    writeJson << m_metadata.getMetadata();
    writeJson.close();

    std::ifstream readWords(m_sPath + "/intern/pages.txt");
    if(!readWords || readWords.peek() == std::ifstream::traits_type::eof() ) {
        createPages();
        createMapPreview();
        safePages();
    }

    else
        loadPages();

    //Test output to see if authors function works
    for(const auto &it : m_metadata.getAuthors())
        std::cout << it << "; ";
    m_hasOcr = true;
}

void CBook::createPages()
{
    std::cout << "Creating map of words... \n";
    std::ifstream read(getOcrPath());

    fs::create_directories(m_sPath + "/intern");

    std::string sBuffer = "";
    size_t pageCounter = 0;

    bool pageMark = false;
    for(size_t i=0; i<10; i++) {
        std::string sLine;
        getline(read, sLine);
        if(func::checkPage(sLine) == true)
            pageMark = true;
    }
    read.clear();
    read.seekg(0, std::ios::beg); 

    size_t page=1;
    size_t blanclines=3;
    std::string convertToNormalLayout = "----- 0 / 999 -----\n\n";
    while(!read.eof()) {

        std::string sLine;
        getline(read, sLine);

        if(sLine == "" && pageMark == false) 
            blanclines++;
        else if(sLine != "" && pageMark == false)
            blanclines = 0;

        //Create books with page mark (old format)
        if(func::checkPage(sLine) == true && pageMark == true) {
            for(auto it : func::extractWordsFromString(sBuffer)) {
                std::get<0>(m_mapWordsPages[it.first]).push_back(page);
                std::get<1>(m_mapWordsPages[it.first]) += it.second*(it.second+1) / 2;
            } 
            pageCounter++;

            //Create new page
            std::ofstream write(m_sPath + "/intern/page" + std::to_string(page) + ".txt");
            write << func::returnToLower(sBuffer);
            write.close();
            sBuffer = "";

            std::string num = sLine.substr(6, sLine.find("/")-7);
            page = stoi(num);
        }
        //Create books without page mark (new format)
        else if( (blanclines == 4 || (blanclines >= 4 && blanclines % 2 == 1)) && pageMark == false)
        {
            //std::cout << "creating page " << page << std::endl;
            for(auto it : func::extractWordsFromString(sBuffer)) {
                std::get<0>(m_mapWordsPages[it.first]).push_back(page);
                std::get<1>(m_mapWordsPages[it.first]) += it.second*(it.second+1) / 2;
            } 
            pageCounter++;

            //Create new page
            std::ofstream write(m_sPath + "/intern/page" + std::to_string(page) + ".txt");
            write << func::returnToLower(sBuffer);
            write.close();

            convertToNormalLayout += "\n\n----- "+std::to_string(page)+" / 999 -----\n\n" + sBuffer;
            sBuffer = "";

            page++;
            //blanclines = 0;
        }
        else
            sBuffer += " " + sLine + "\n";
    }
    if(sBuffer.length() !=0)
    {
        for(auto it : func::extractWordsFromString(sBuffer)) {
            std::get<0>(m_mapWordsPages[it.first]).push_back(page);
            std::get<1>(m_mapWordsPages[it.first]) += it.second*(it.second+1) / 2;
        } 

        //Create new page
        std::ofstream write(m_sPath + "/intern/page" + std::to_string(page) + ".txt");
        write << func::returnToLower(sBuffer);
        write.close();

        if(pageMark == false)
            convertToNormalLayout +="\n\n----- "+std::to_string(page)+" / 999 -----\n\n" + sBuffer;
    }

    read.close();
    if(pageMark == false)
    {
        try {
            std::filesystem::rename(m_sPath + "/ocr.txt", m_sPath + "/old_ocr.txt");
        } catch (std::filesystem::filesystem_error& e) {
            std::cout << e.what() << "\n";
        }
        std::ofstream write (m_sPath + "/ocr.txt");
        write << convertToNormalLayout;
        write.close();
    }
    m_numPages = pageCounter;
}

void CBook::createMapPreview()
{
    std::cout << "Create map Prev." << std::endl;
    for(auto it : m_mapWordsPages) {
        std::get<2>(m_mapWordsPages[it.first]) = getPreviewPosition(it.first);
    }
}
    

/**
* @brief safe map of all words and pages on which word occures to disc
*/
void CBook::safePages()
{
    std::cout << "Saving pages" << std::endl;
    std::ofstream write(m_sPath + "/intern/pages.txt");
    write << m_numPages << "\n";

    for(auto it : m_mapWordsPages)
    {
        std::string sWord = it.first;
        std::string sBuffer = func::convertStr(sWord);
        sBuffer += ";";
        for(size_t page : std::get<0>(it.second)) {
            sBuffer += std::to_string(page) + ",";
        }
        sBuffer += ";" + std::to_string(std::get<1>(m_mapWordsPages[it.first]));
        sBuffer += ";" + std::to_string(std::get<2>(m_mapWordsPages[it.first]));
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
    std::cout << m_sKey << "Loading pages." << std::endl;
    //Load map
    std::ifstream read(m_sPath + "/intern/pages.txt");
    std::string sBuffer;

    //Read number of pages
    getline(read, sBuffer);

    if(std::isdigit(sBuffer[0]) == false) {
        createPages();
        createMapPreview();
        safePages();
        return;
    }

    m_numPages = stoi(sBuffer);

    //Read words, pages, and relevance, preview-position
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
        m_mapWordsPages[vec[0]] = make_tuple(pages, std::stoi(vec[2]), std::stoi(vec[3]));
    }

    read.close();
}


// **** GET PAGES FUNCTIONS **** //

/**
* @brief getPages calls the matching getPages... function according to fuzzyness
*/
std::map<int, std::vector<std::string>>* CBook::getPages(std::string sInput, bool fuzzyness)
{
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;
    if(m_hasOcr == false)
        return mapPages;

    std::replace(sInput.begin(), sInput.end(), ' ', '+');
    std::vector<std::string> vWords = func::split2(func::returnToLower(sInput), "+");

    //Create map of pages and found words for first word
    mapPages = findPages(vWords[0], fuzzyness);

    for(size_t i=1; i<vWords.size(); i++) {

        std::map<int, std::vector<std::string>>* mapPages2 = findPages(vWords[i], fuzzyness);

        //Remove all elements from mapPages, which do not exist in results2. 
        removePages(mapPages, mapPages2);
        delete mapPages2;
    }

    std::cout << "Found hits for " << sInput <<  " on " << mapPages->size() << " pages.\n";

    return mapPages;
}


//Create map of pages and found words for i-word (full-search)
std::map<int, std::vector<std::string>>* CBook::findPages(std::string sWord, bool fuzzyness)
{
    //Create empty list of pages
    std::map<int, std::vector<std::string>>* mapPages = new std::map<int, std::vector<std::string>>;

    //Fuzzyness = false
    if (fuzzyness == false) {
        if(m_mapFull.count(sWord) > 0) {
            for(auto elem : m_mapFull[sWord]) {
                for(auto page : std::get<0>(m_mapWordsPages.at(elem)))
                    (*mapPages)[page].push_back(elem);
            }
        }
        else if(m_mapWordsPages.count(sWord) > 0) {
            for(auto page : std::get<0>(m_mapWordsPages.at(sWord)))
                (*mapPages)[page].push_back(sWord);
        }
    }

    //Fuzzyness = true
    else
    {
        //1. try map of fuzzy matches
        if(m_mapFuzzy.count(sWord) > 0) {
            for(auto elem : m_mapFuzzy[sWord]) {
                for(auto page : std::get<0>(m_mapWordsPages.at(elem.first)))
                    (*mapPages)[page].push_back(elem.first);
            }
        }

        //2. Iterate over list of words
        else {
            for(auto it : m_mapWordsPages) {
                if(fuzzy::fuzzy_cmp(it.first, sWord) <= 0.2) {
                    for(auto page : std::get<0>(it.second))
                        (*mapPages)[page].push_back(it.first);
                }
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
    for(auto it=results1->begin(); it!=results1->end();)
    {
        if(results2->count(it->first) == 0)
            it = results1->erase(it);
        else {
            it->second.insert(it->second.end(), (*results2)[it->first].begin(), (*results2)[it->first].end());
            ++it;
        }
    }
}

bool CBook::onSamePage(std::vector<std::string> sWords)
{
    std::vector<size_t> pages1 = pages(sWords[0]);
    if(pages1.size() == 0) return false;

    for(size_t i=1; i<sWords.size(); i++)
    {
        
        std::vector<size_t> pages2 = pages(sWords[i]);

        if(pages2.size() == 0) return false;

        bool found=false;
        for(size_t j=0; j<pages1.size(); j++) {
            if(std::find(pages2.begin(), pages2.end(), pages1[j]) != pages2.end()) {
                found=true;
                break;
            }
        }

        if(found==false) 
            return false;
    }
    
    return true;
}

std::vector<size_t> CBook::pages(std::string sWord) {
    std::vector<size_t> pages;

    if(m_mapWordsPages.count(sWord) > 0)
        pages = std::get<0>(m_mapWordsPages[sWord]);
    else if(m_mapFuzzy.count(sWord) > 0) {
        for(auto elem : m_mapFuzzy[sWord]) {
            std::vector<size_t> foo = std::get<0>(m_mapWordsPages.at(elem.first));
            pages.insert(pages.begin(), foo.begin(), foo.end());
        }
    }
    return pages;
}


// **** GET PREVIEW FUNCTIONS **** //
std::string CBook::getPreview(std::string sInput)
{
    // *** Get First preview *** //
    std::vector<std::string> searchedWords = func::split2(sInput, "+");
    std::string prev = getOnePreview(searchedWords[0]);

    // *** Get second Preview (if second word) *** //
    for(size_t i=1; i<searchedWords.size(); i++)
    {
        //Try finding second word in current preview
        size_t pos = prev.find(searchedWords[i]);
        if(pos!=std::string::npos) {
            size_t end = prev.find(" ", pos);
            if(end != std::string::npos) {
                prev.insert(end, "</mark>");
            }
            else
                prev.insert(pos+searchedWords[i].length(), "</mark>");
            prev.insert(pos, "<mark>");
        } 
        else
            prev += "\n" + getOnePreview(searchedWords[i]);
    }
    return prev;
}

/**
* @brief get a preview of the page where the searched word has been found
* @param sWord (searched word)
* @param fuzzyness
* @return Preview
*/
std::string CBook::getOnePreview(std::string sWord)
{
    std::string sSource;
    size_t pos;
    size_t page=1000000;

    // *** get Source string and position *** //
    if(m_hasOcr == true) 
        sSource = getPreviewText(sWord, pos, page);
    else
        sSource = getPreviewTitle(sWord, pos);

    if(sSource=="") return "No Preview.";

    // *** Generate substring *** //
    size_t front = 0;
    size_t len = 150;
    if(pos > 75) front = pos - 75;
    if(front+len >= sSource.length()) len = sSource.length()-front;

    std::string finalResult = sSource.substr(front, len);

    // *** Add highlighting *** //
    if (pos > 75) pos = 75;
    size_t end = finalResult.find(" ", pos+1);
    if(end!=std::string::npos)
        finalResult.insert(end, "</mark>");
    else
        finalResult.insert(pos+sWord.length(), "</mark>");
    finalResult.insert(pos, "<mark>");

    //*** Shorten preview if needed *** //
    shortenPreview(finalResult);

    //*** Append [...] front and back *** //
    finalResult.insert(0, " \u2026"); 
    if(page!=1000000)
        return finalResult += " \u2026 (S." + std::to_string(page) +")";
    else
        return finalResult += " \u2026";
}

std::string CBook::getPreviewText(std::string& sWord, size_t& pos, size_t& page)
{
    // *** get match *** //
    if(m_mapWordsPages.count(sWord) > 0)
        sWord = sWord;
    else if(m_mapFull.count(sWord) > 0)
        sWord = m_mapFull[sWord].front();
    else if(m_mapFuzzy.count(sWord) > 0) 
        sWord = m_mapFuzzy[sWord].front().first;
    else 
        return "";

    // *** Get Page *** //
    page = std::get<0>(m_mapWordsPages[sWord])[0];
    if(page == 1000000)
        return "";

    // *** Get Source string *** //
    std::ifstream read(m_sPath + "/intern/page" + std::to_string(page) + ".txt", std::ios::in);
    std::string sSource((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());

    // *** Get Pos *** //
    pos = std::get<2>(m_mapWordsPages[sWord]);

    return sSource;
}

std::string CBook::getPreviewTitle(std::string& sWord, size_t& pos)
{
    // *** Get Source string *** //
    std::string sTitle = m_metadata.getTitle() + " ";
    func::convertToLower(sTitle);

    // *** Find Pos *** //
    pos = sTitle.find(sWord);
    if(pos == std::string::npos || sTitle == "" || pos > sTitle.length())
    {
        for(auto it : func::split2(sTitle, " ")) {
            if(fuzzy::fuzzy_cmp(func::convertStr(it), sWord) < 0.2) {
                pos=sTitle.find(it);
                if(pos != std::string::npos && sTitle != "" && pos <= sTitle.length())
                    return sTitle;
            }
        }
        return "";
    }
    return sTitle;
}


/**
* @brief Find preview with matched word (best match), and page on which the match was found.
* @param[in] sWord (best Match)
* @param[in] page (page on which match was found)
* @return preview for this book
*/
size_t CBook::getPreviewPosition(std::string sWord)
{
    std::vector<size_t> pages = std::get<0>(m_mapWordsPages[sWord]);
    for(size_t i=0; i<pages.size(); i++) 
    {
        //Read ocr and kontent
        std::ifstream read(m_sPath + "/intern/page" + std::to_string(pages[i]) + ".txt", std::ios::in);
        std::string sPage((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());

        //Find match
        size_t pos = sPage.find(sWord);

        //Return "error" if not found
        if (pos != std::string::npos)
            return pos;
    }
    
    return 1000000;
}


void CBook::shortenPreview(std::string& str)
{
    //Delete invalid chars front
    for(;;)
    {
        if((int)str.front() < 0)
            str.erase(0,1);
        else if((int)str.back() < 0)
            str.pop_back();
        else
            break;
    }

    //Check vor invalid literals and escape
    for(unsigned int i=str.length(); i>0; i--)
    {
        if(str[i-1] == '\"' || str[i-1] == '\'' || str[i-1] == '\\')
            str.erase(i-1, 1);
        if(str[i-1] == '<' && str[i] != 'm' && str[i] != '/')
            str.erase(i-1, 1);
    }
}

void CBook::addPage(std::string sInput, std::string sPage, std::string sMaxPage)
{
    std::ofstream writePage(m_sPath+"/intern/add"+sPage+".txt");
    writePage << sInput;
    writePage.close();

    fs::path p = getOcrPath();
    fs::exists(p);
    std::string sSource;
    if(fs::exists(p) == false) {
        std::cout << "create new ocr..." << std::endl;
        sInput.insert(0, "\n----- "+sPage+" / "+ sMaxPage +" -----\n"); 
        sSource = sInput;
    }

    else {
        std::map<int, std::string> orderMap;
        std::cout << "Adding to existing ocr ... " << std::endl;
        for(auto& p : fs::directory_iterator(m_sPath+"/intern")) {
            std::string filename=p.path().filename();
            if(filename.find("add")!=std::string::npos) {
                std::string num = filename.substr(3, filename.length()-2-filename.find("."));
                orderMap[stoi(num)] = p.path();
            }
        }

        for(auto it : orderMap) {
            sSource.append("\n\n----- "+std::to_string(it.first)+" / "+ sMaxPage +" -----\n");
            std::ifstream r2(it.second);
            std::string str((std::istreambuf_iterator<char>(r2)), std::istreambuf_iterator<char>());
            sSource.append(str);
        }
    }

    std::cout << sSource << std::endl;
    
    std::ofstream writeOcr(m_sPath + "/ocr.txt");
    writeOcr << sSource;
    writeOcr.close();
}
