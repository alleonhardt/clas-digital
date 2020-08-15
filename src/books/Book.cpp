#include "CBook.hpp"
#include <ctime>

namespace fs = std::filesystem;

CBook::CBook () {}

/**
* @Brief Constructor
* @param[in] sPath Path to book
* @param[in] map map of words in book
*/
CBook::CBook(nlohmann::json jMetadata) : metadata_(jMetadata)
{
  key_ = jMetadata["key"];
  path_ = "web/books/"+key_;
  has_ocr_ = false;
  has_images_ = false;

  //Metadata
  author_ = metadata_.GetAuthor();                 
  if(author_.size() == 0) author_ = "No Author";
  author_date_ = author_;
  func::convertToLower(author_);
  date_    = metadata_.GetDate();
  collections_ = metadata_.GetCollections();
  if(date_ != -1) author_date_ += ", " + std::to_string(date_);
  author_date_ += ".";
}

// **** GETTER **** //

const std::string& CBook::get_key() { 
  return key_;
}

const std::string& CBook::get_path() {
  return path_;
}

std::string CBook::get_ocr_path() {
  return path_ + "/ocr.txt";
}

bool CBook::has_ocr() const { 
  return has_ocr_;
}

bool CBook::has_images() const { 
  return has_images_; }

int CBook::get_num_pages() { 
  return num_pages_; 
}

MetadataHandler& CBook::get_metadata() { 
  return metadata_; 
} 

std::string CBook::get_author() { 
  return author_; 
}

int CBook::get_date() { 
  return date_; 
}

std::vector<std::string> CBook::get_collections() { 
  return collections_; 
}

std::string CBook::get_author_date() { 
  return author_date_; 
}


///Get map of words with list of pages, preview-position and relevance.
std::unordered_map<std::string, std::tuple<std::vector<size_t>,int,size_t>>& 
CBook::get_map_words_pages() { 
  return map_words_pages_;
}

///Return matches found with fuzzy-search
std::unordered_map<std::string, std::list<std::pair<std::string, double>>>& 
CBook::get_found_fuzzy_matches() {
  return found_fuzzy_matches_;
}

///Return matches found with contains-search
std::unordered_map<std::string, std::list<std::string>>& CBook::get_found_grammatical_matches() {
  return found_grammatical_matches_;
}

///Return whether book has images or ocr
bool CBook::hasContent() const { 
  return has_images_ || has_ocr_; 
}

///Return whether book has title, author or date
bool CBook::checkJson() {
  if(author_ == "" && metadata_.GetTitle() == "" && date_ == -1)
   return false;
  return true;
}

///Return "[author], [date]" and add "book not yet scanned", when has_ocr == false
std::string CBook::getAuthorDateScanned() {
  if(has_ocr_ == true)
    return author_date_;
  return author_date_ + "<span style='color:orange;font-size:80%'> Book is not yet scanned, sorry for that.</span>";
}    

///Return whether book is publicly accessible 
bool CBook::getPublic() {
  std::time_t ttime = time(0);
  tm *local_time = localtime(&ttime);

  if(metadata_.GetMetadata("rights","data") == "CLASfrei")
    return true;

  //Local time number of seconds elapsed since 1. January 1900. 
  if(get_date() == -1 || get_date() >= (local_time->tm_year+1800))
    return false;
  else
    return true;
}
// **** SETTER **** //

void CBook::setPath(std::string sPath) { 
    path_ = sPath; 
}


// **** CREATE BOOK AND MAPS (PAGES, RELEVANCE, PREVIEW) **** // 

/**
* @brief sets has_ocr_/Images, 
* if has_ocr==true, safes json to disc creates/ loads map of words/pages.
* @param[in] sPath (path to book)
*/
void CBook::createBook(std::string sPath)
{
    //Check if book has images
    for(auto& p: std::filesystem::directory_iterator(sPath))
    {
	    (void)p;
        if(p.path().extension() == ".jpg" || p.path().extension() == ".bmp")
            has_images_ = true; 
    }

    //Open ocr, if it doesn't exist, end function -> book does not need to be "created".
    if(!std::filesystem::exists(get_ocr_path())) return;
    has_ocr_ = true;

    //Write json to disc.
    std::ofstream writeJson(path_ + "/info.json");
    writeJson << metadata_.get_json();
    writeJson.close();

    //Check whether list of words_pages already exists, if yes load, otherwise, create
    std::string path = path_ + "/intern/pages.txt";
    //if(!std::filesystem::exists(path) || std::filesystem::is_empty(path))
    //{
        //Create pages
        CreatePages();
        //Find (first) preview.
        CreateMapPreview();
        //Safe to disc
        SafePages();
    //}
    //Load pages
    //else
        //LoadPages();
}

///Create map of all pages and safe.
void CBook::CreatePages()
{
    std::cout << "Creating map of words... \n";
    //Load ocr and create ne directory "intern" for this book.
    std::ifstream read(get_ocr_path());
    fs::create_directories(path_ + "/intern");

    std::string sBuffer = "", sLine = "";

    //Check if book has a mark for end/ beginning of page.
    bool pageMark = false;
    for(size_t i=0; i<10; i++) 
    {
        std::getline(read, sLine);
        //check page, checks whether current line is of page-break-form
        if(func::checkPage(sLine) == true) {
            pageMark = true;
            break;
        }
    }
    //Reset file
    read.clear();
    read.seekg(0, std::ios::beg); 

    //Variable storing complete ocr, to convert to page-break-format if necessary 
    std::string sConvertToNormalLayout = "----- 0 / 999 -----\n\n";
    size_t page=1, pageCounter = 0, blanclines = 3;
    while(!read.eof()) 
    {
        //Read line
        getline(read, sLine);

        //Increase, or reset blanc lines
        if(sLine == "") blanclines++;
        else blanclines = 0;

        //Add a page when new page is reached.
        if((func::checkPage(sLine) == true && pageMark == true) || ((blanclines == 4 || (blanclines >= 4 && blanclines % 2 == 1)) && pageMark == false))
        {
            //Add new page to map of pages and update values. Safe page to disc
            CreatePage(sBuffer, sConvertToNormalLayout, page, pageMark);

            //Different handling for page-break- and non-page-break-(new)-format
            if(pageMark == true)   
                page = stoi(sLine.substr(6, sLine.find("/")-7));
            else
                page++; 
            sBuffer = "";
            pageCounter++;
        }
        
        //Append line to buffer if page end is not reached.
        else
            sBuffer += " " + sLine + "\n";
    }

    //If there is "something left", safe as new page.
    if(sBuffer.length() !=0)
        CreatePage(sBuffer, sConvertToNormalLayout, page, pageMark);

    read.close();

    //Write page-mark-format to disc, if ocr was in new format.
    if(pageMark == false)
    {
        //Move new format, so it is not overwritten.
        try {
            std::filesystem::rename(path_ + "/ocr.txt", path_ + "/old_ocr.txt");
        } catch (std::filesystem::filesystem_error& e) {
            std::cout << e.what() << "\n";
        }

        //Safe as page-break-format
        std::ofstream write (path_ + "/ocr.txt");
        write << sConvertToNormalLayout;
        write.close();
    }

    //Set number of pages for this book.
    num_pages_ = pageCounter;
}


/**
* @brief function adding all words from one page to map of words. Writes the
* page to disc as single file. Add the page break line to a string used to convert 
* new format to old/ normal format. 
* @param[in] sBuffer (string holding current page)
* @param[in, out] sConvert (string holding copy of complete ocr in page-mark-format)
* @param[in] page (number indexing current page)
* @param[in] mark (page-mark-format yes/no)
*/
void CBook::CreatePage(std::string sBuffer, std::string& sConvert, size_t page, bool mark)
{
    //Add entry, or update entry in map of words/ pages (new page + relevance)
    for(auto it : func::extractWordsFromString(sBuffer)) {
        std::get<0>(map_words_pages_[it.first]).push_back(page);
        std::get<1>(map_words_pages_[it.first]) += it.second*(it.second+1) / 2;
    } 

    //Create new page
    std::ofstream write(path_ + "/intern/page" + std::to_string(page) + ".txt");
    write << func::returnToLower(sBuffer);
    write.close();

    if(mark == false)
        sConvert += "\n\n----- "+std::to_string(page)+" / 999 -----\n\n" + sBuffer;
}

/**
* @brief Find preview position for each word in map of words/pages.
*/
void CBook::CreateMapPreview()
{
    std::cout << "Create map Prev." << std::endl;
    for(auto it : map_words_pages_) {
        std::get<2>(map_words_pages_[it.first]) = GetPreviewPosition(it.first);
    }
}
    

/**
* @brief safe map of all words and pages to disc
*/
void CBook::SafePages()
{
    std::cout << "Saving pages" << std::endl;
    //Open file.
    std::ofstream write(path_ + "/intern/pages.txt");
    write << num_pages_ << "\n";

    //Iterate over map of words
    for(auto it : map_words_pages_)
    {
        //Write word in converted format (replace a by ä, é by e ...)
        std::string sWord = it.first;
        std::string sBuffer = func::convertStr(sWord) + ";";

        //Add all pages word occurs on
        for(size_t page : std::get<0>(it.second)) {
            sBuffer += std::to_string(page) + ",";
        }

        //Add preview-position and relevance.
        sBuffer += ";" + std::to_string(std::get<1>(map_words_pages_[it.first]));
        sBuffer += ";" + std::to_string(std::get<2>(map_words_pages_[it.first])) + "\n";

        write << sBuffer;
    }
    write.close();
}


/**
* @brief load words and pages on which word occurs into map
*/
void CBook::LoadPages()
{
    std::cout << key_ << "Loading pages." << std::endl;
    //Load map
    std::ifstream read(path_ + "/intern/pages.txt");
    std::string sBuffer;

    //Read number of pages, if first lind does not indicate pages, recreate book.
    getline(read, sBuffer);
    if(std::isdigit(sBuffer[0]) == false) {
        CreatePages();
        CreateMapPreview();
        SafePages();
        return;
    }
    num_pages_ = stoi(sBuffer);

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
        for(auto page : func::split2(vec[1], ",")) 
        {
            //Check if page actually is number, then add to pages
            if(isdigit(page[0]))
                pages.push_back(std::stoi(page));
        }

        //Add word and pages to map
        map_words_pages_[vec[0]] = make_tuple(pages, std::stoi(vec[2]), std::stoi(vec[3]));
    }

    read.close();
}


// **** GET PAGES FUNCTIONS **** //


/**
* @brief getPages calls FindPages (extracting all pages from given word) for each 
* word searched and removes duplicates and/ or pages, where not all words searched
* occur.
* @param[in] sInput (list of searched words as a string, separated by ' ' or + 
* @param[in] fuzzyness (boolean indicating whether fuzziness is set or not)
* @return map of pages, with vector of words on this page 
* (all the same if fuzziness==false)
*/
std::map<int, std::vector<std::string>>* CBook::getPages(std::string sInput, bool fuzzyness)
{
    //Initialize new map and return empty map in case the book has no ocr.
    auto* mapPages = new std::map<int, std::vector<std::string>>;
    if(has_ocr_ == false)
        return mapPages;

    //Do some parsing, as user can use ' ' or + to indicate searching for several words.
    std::replace(sInput.begin(), sInput.end(), ' ', '+');
    std::vector<std::string> vWords = func::split2(func::returnToLower(sInput), "+");

    //Create map of pages and found words for first word
    mapPages = FindPages(vWords[0], fuzzyness);

    //Iterate over all 1..n words create list of pages, and remove words not on same page.
    for(size_t i=1; i<vWords.size(); i++) 
    {
        //Get pages from word i.
        auto* mapPages2 = FindPages(vWords[i], fuzzyness);

        //Remove all elements from mapPages, which do not exist in results2. 
        RemovePages(mapPages, mapPages2);
        delete mapPages2;
    }
    return mapPages;
}

/**
* @brief Create map of pages and found words on page. As words found may differ 
* from searched word. (F.e. "Löwe" may match for "Löwin" even if fuzziness == false).
* @param[in] sWord (word search)
* @param[in] fuzzyness (boolean indicating whether fuzziness is set or not)
* @return map of all pages on which word was found.
*/
std::map<int, std::vector<std::string>>* CBook::FindPages(std::string sWord, bool fuzzyness)
{
    //Create empty list of pages
    auto* mapPages = new std::map<int, std::vector<std::string>>;

    //Normal-search
    if (fuzzyness == false) 
    {
        //Check for different grammatical forms.
        if(found_grammatical_matches_.count(sWord) > 0) {
            //Obtain pages from each word.
            for(auto elem : found_grammatical_matches_[sWord]) {
                for(auto page : std::get<0>(map_words_pages_.at(elem)))
                    (*mapPages)[page].push_back(elem);
            }
        }
        //Obtains pages.
        else if(map_words_pages_.count(sWord) > 0) {
            for(auto page : std::get<0>(map_words_pages_.at(sWord)))
                (*mapPages)[page].push_back(sWord);
        }
    }

    //Fuzziness-search 
    else
    {
        //Check for words in map of fuzzy matches.
        if(found_fuzzy_matches_.count(sWord) > 0) {
            //Obtain pages from each word found by fuzzy-search.
            for(auto elem : found_fuzzy_matches_[sWord]) {
                for(auto page : std::get<0>(map_words_pages_.at(elem.first)))
                    (*mapPages)[page].push_back(elem.first);
            }
        }
    }
    return mapPages;
}


/**
* @brief Remove all elements from results-1, which do not exist in results-2. 
* @param[in, out] results1
* @param[in] results2
* @return map of pages and words found on this page
*/
void CBook::RemovePages(std::map<int, std::vector<std::string>>* results1, std::map<int, std::vector<std::string>>* results2)
{
    //Iterate over results-1.
    for(auto it=results1->begin(); it!=results1->end();)
    {
        //Erase if, page does not exist in results-2
        if(results2->count(it->first) == 0)
            it = results1->erase(it);
        //Insert words on page i in results-2 into words on page i in results-1
        else {
            std::vector<std::string> vec = (*results2)[it->first];
            it->second.insert(it->second.end(), vec.begin(), vec.end());
            ++it;
        }
    }
}

/**
* @brief checks whether words found occur on the same page
* @param[in] sWords (words to check)
* @return boolean indicating whether words or on the same page or not
*/
bool CBook::onSamePage(std::vector<std::string> vWords, bool fuzzyness)
{
    //Check if words occur only in metadata (Author, Title, Date)
    if(MetadataCmp(vWords, fuzzyness) == true)
        return true;

    //Get all pages, the first word is on
    std::vector<size_t> pages1 = PagesFromWord(vWords[0], fuzzyness);

    //If no pages or found, return false
    if(pages1.size() == 0) 
        return false;

    //Iterate over words 1..n
    for(size_t i=1; i<vWords.size(); i++)
    {
        //get all pages of word i.
        std::vector<size_t> pages2 = PagesFromWord(vWords[i], fuzzyness);

        //Return false if pages are empty
        if(pages2.size() == 0) 
            return false;

        //Check if all pages in pages-1 occur in pages-2.
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

/** 
* @brief check whether all words occur in metadata
* @param[in] vWords (words to check)
* @param[in] fuzzyness (fuzzy-search yes/ no)
* @return boolean whether all words are in metadata or not.
*/
bool CBook::MetadataCmp(std::vector<std::string> vWords, bool fuzzyness)
{
    bool inMetadata = true;
            
    //Iterate over all words and check for occurrence in metadata
    for(const auto& word : vWords) {
        std::string sTitle = metadata_.GetTitle(); func::convertToLower(sTitle);
        std::string sAuthor = metadata_.GetAuthor(); func::convertToLower(sAuthor);

        //If word occurs neither in author, title, or date, set inMetadata to false.
        if(sAuthor.find(word) == std::string::npos && sTitle.find(word) == std::string::npos && std::to_string(date_) != word) 
            inMetadata = false;

        //Check title and author again with fuzzy search.
        if(fuzzyness == true)
        {
            //Check title
            bool found = false;
            sTitle = func::convertStr(sTitle);
            for(auto it : func::extractWordsFromString(sTitle)) {
                if(fuzzy::fuzzy_cmp(word, it.first) <= 0.2)
                    found = true;
            }

            //check author
            sAuthor = func::convertStr(sAuthor);
            for(auto it : func::extractWordsFromString(sAuthor)) {
                if(fuzzy::fuzzy_cmp(word, it.first) <= 0.2)
                    found = true;
            }
            
            //if found with fuzzy-search, set check-variable to true.
            if(found == true)
                inMetadata = found;
        }

        if(inMetadata == false)
            return false;
    }
    return inMetadata;
}

/**
* @brief generates list of all pages from searched word, also checking fuzzy-matches
* (but not grammatical matches (please check!!!))
* @param[in] word to generate list of pages for.
* @return list of pages
*/
std::vector<size_t> CBook::PagesFromWord(std::string sWord, bool fuzzyness)
{
    //Use set to automatically erase duplicates.
    std::set<size_t> pages;

    //Obtain pages with exact match
    if(map_words_pages_.count(sWord) > 0)
    {
        std::vector<size_t> foo = std::get<0>(map_words_pages_.at(sWord));
        pages.insert(foo.begin(), foo.end());
    }

    //Obtain pages from grammatical matches
    if(found_grammatical_matches_.count(sWord) > 0)
    {
        for(auto elem : found_grammatical_matches_[sWord]) {
            std::vector<size_t> foo = std::get<0>(map_words_pages_.at(elem));
            pages.insert(foo.begin(), foo.end());
        }
    }

    //Obtain pages from fuzzy match.
    if(found_fuzzy_matches_.count(sWord) > 0 && fuzzyness == true) 
    {
        for(auto elem : found_fuzzy_matches_[sWord]) {
            std::vector<size_t> foo = std::get<0>(map_words_pages_.at(elem.first));
            pages.insert(foo.begin(), foo.end());
        }
    }
    
    //convert set to vector.
    std::vector<size_t> vec;
    vec.assign(pages.begin(), pages.end());
    return vec;
}


// **** GET PREVIEW FUNCTIONS **** //

std::string CBook::getPreview(std::string sInput)
{
    // *** Get First preview *** //
    std::vector<std::string> searchedWords = func::split2(sInput, "+");
    std::string prev = GetOnePreview(searchedWords[0]);

    // *** Get second Preview (if second word) *** //
    for(size_t i=1; i<searchedWords.size(); i++)
    {
        //Try finding second word in current preview
        size_t pos = prev.find(searchedWords[i]);
        if(pos!=std::string::npos) {
            size_t end = prev.find(" ", pos);
            if(end != std::string::npos) 
                prev.insert(end, "</mark>");
            else
                prev.insert(pos+searchedWords[i].length(), "</mark>");
            prev.insert(pos, "<mark>");
        } 
        else
        {
            std::string newPrev = GetOnePreview(searchedWords[i]);
            if(newPrev != "No Preview.")
                prev += "\n" + newPrev;
        }
    }
    return prev;
}

/**
* @brief get a preview of the page where the searched word has been found
* @param sWord (searched word)
* @param fuzzyness
* @return Preview
*/
std::string CBook::GetOnePreview(std::string sWord)
{
    std::string sSource;
    size_t pos;
    size_t page=1000000;

    // *** get Source string and position *** //
    if(has_ocr_ == true) 
        sSource = GetPreviewText(sWord, pos, page);
    else
        sSource = GetPreviewTitle(sWord, pos);

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
    ShortenPreview(finalResult);

    //*** Append [...] front and back *** //
    finalResult.insert(0, " \u2026"); 
    if(page!=1000000)
        return finalResult += " \u2026 (S. " + std::to_string(page) +")";
    else
        return finalResult += " \u2026";
}

std::string CBook::GetPreviewText(std::string& sWord, size_t& pos, size_t& page)
{
    // *** get match *** //
    if(map_words_pages_.count(sWord) > 0)
        sWord = sWord;
    else if(found_grammatical_matches_.count(sWord) > 0)
    {
        sWord = found_grammatical_matches_[sWord].front();
        if(map_words_pages_.count(sWord) == 0)
        {
            std::cout << "Word in mapFull, which is not in mapWordPages.\n";
            return "";
        }
    }
    else if(found_fuzzy_matches_.count(sWord) > 0) 
        sWord = found_fuzzy_matches_[sWord].front().first;
    else 
        return "";

    // *** Get Page *** //
    page = std::get<0>(map_words_pages_[sWord])[0];
    if(page == 1000000)
        return "";

    // *** Get Source string *** //
    std::ifstream read(path_ + "/intern/page" + std::to_string(page) + ".txt", std::ios::in);
    std::string sSource((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());

    // *** Get Pos *** //
    pos = std::get<2>(map_words_pages_[sWord]);

    return sSource;
}

std::string CBook::GetPreviewTitle(std::string& sWord, size_t& pos)
{
    // *** Get Source string *** //
    std::string sTitle = metadata_.GetTitle() + " ";
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
size_t CBook::GetPreviewPosition(std::string sWord)
{
    std::vector<size_t> pages = std::get<0>(map_words_pages_[sWord]);
    for(size_t i=0; i<pages.size(); i++) 
    {
        //Read ocr and kontent
        std::ifstream read(path_ + "/intern/page" + std::to_string(pages[i]) + ".txt", std::ios::in);
        std::string sPage((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());

        //Find match
        size_t pos = sPage.find(sWord);

        //Return "error" if not found
        if (pos != std::string::npos)
            return pos;
    }
    
    return 1000000;
}


void CBook::ShortenPreview(std::string& str)
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
    std::ofstream writePage(path_+"/intern/add"+sPage+".txt");
    writePage << sInput;
    writePage.close();

    fs::path p = get_ocr_path();
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
        for(auto& p : fs::directory_iterator(path_+"/intern")) {
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
    
    std::ofstream writeOcr(path_ + "/ocr.txt");
    writeOcr << sSource;
    writeOcr.close();
}
