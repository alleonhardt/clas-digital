#include "book.h"
#include <ctime>

namespace fs = std::filesystem;

Book::Book () {}

/**
* @Brief Constructor
* @param[in] sPath Path to book
* @param[in] map map of words in book
*/
Book::Book(nlohmann::json jMetadata) : metadata_(jMetadata)
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

const std::string& Book::key() { 
  return key_;
}

const std::string& Book::path() {
  return path_;
}

std::string Book::ocr_path() {
  return path_ + "/ocr.txt";
}

bool Book::has_ocr() const { 
  return has_ocr_;
}

bool Book::has_images() const { 
  return has_images_; }

int Book::num_pages() { 
  return num_pages_; 
}

MetadataHandler& Book::metadata() { 
  return metadata_; 
} 

std::string Book::author() { 
  return author_; 
}

int Book::date() { 
  return date_; 
}

std::vector<std::string> Book::collections() { 
  return collections_; 
}

std::string Book::author_date() { 
  return author_date_; 
}


///Get map of words with list of pages, preview-position and relevance.
std::unordered_map<std::string, WordInfo>& Book::map_words_pages() { 
  return map_words_pages_;
}

///Return matches found with fuzzy-search
std::unordered_map<std::string, std::list<std::pair<std::string, double>>>& 
Book::found_fuzzy_matches() {
  return found_fuzzy_matches_;
}

///Return matches found with contains-search
std::unordered_map<std::string, std::list<std::string>>& Book::found_grammatical_matches() {
  return found_grammatical_matches_;
}

///Return whether book has images or ocr
bool Book::HasContent() const { 
  return has_images_ || has_ocr_; 
}

///Return "[author], [date]" and add "book not yet scanned", when has_ocr == false
std::string Book::GetAuthorDateScanned() {
  if(has_ocr_ == true)
    return author_date_;
  return author_date_ + "<span style='color:orange;font-size:80%'> Book is not yet scanned, sorry for that.</span>";
}    

// **** SETTER **** //

void Book::SetPath(std::string sPath) { 
  path_ = sPath; 
}


// **** CREATE BOOK AND MAPS (PAGES, RELEVANCE, PREVIEW) **** // 

/**
* @brief sets has_ocr_/Images, 
* if has_ocr==true, safes json to disc creates/ loads map of words/pages.
* @param[in] sPath (path to book)
*/
void Book::CreateBook(std::string sPath) {
  //Check if book has images
  for (auto& p: std::filesystem::directory_iterator(sPath)) {
    (void)p;
    if(p.path().extension() == ".jpg" || p.path().extension() == ".bmp")
      has_images_ = true; 
  }

  //Open ocr, if it doesn't exist, end function -> book does not need to be "created".
  if (!std::filesystem::exists(ocr_path())) 
    return;
  has_ocr_ = true;

  //Write json to disc.
  std::ofstream writeJson(path_ + "/info.json");
  writeJson << metadata_.GetJson();
  writeJson.close();

  //Check whether list of words_pages already exists, if yes load, otherwise, create
  std::string path = path_ + "/intern/pages.txt";
  if(!std::filesystem::exists(path) || std::filesystem::is_empty(path)) {
    //Create pages
    CreatePages();
    //Find (first) preview.
    CreateMapPreview();
    //Safe to disc
    SafePages();
  }
  //Load pages
  else
    LoadPages();
}

///Create map of all pages and safe.
void Book::CreatePages() {
  std::cout << "Creating map of words... \n";
  //Load ocr and create ne directory "intern" for this book.
  std::ifstream read(ocr_path());
  fs::create_directories(path_ + "/intern");

  std::string sBuffer = "", sLine = "";

  //Check if book has a mark for end/ beginning of page.
  bool pageMark = false;
  for (size_t i=0; i<10; i++) {
    std::getline(read, sLine);
    //check page, checks whether current line is of page-break-form
    if (func::checkPage(sLine) == true) {
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
  while (!read.eof()) {
    //Read line
    getline(read, sLine);

    //Increase, or reset blanc lines
    if (sLine == "") 
      blanclines++;
    else 
      blanclines = 0;

    //Add a page when new page is reached.
    if ((func::checkPage(sLine) == true && pageMark == true) 
        || ((blanclines == 4 || (blanclines >= 4 && blanclines % 2 == 1)) 
          && pageMark == false)) {
      //Add new page to map of pages and update values. Safe page to disc
      CreatePage(sBuffer, sConvertToNormalLayout, page, pageMark);

      //Different handling for page-break- and non-page-break-(new)-format
      if (pageMark == true)   
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
  if (sBuffer.length() !=0)
    CreatePage(sBuffer, sConvertToNormalLayout, page, pageMark);

  read.close();

  //Write page-mark-format to disc, if ocr was in new format.
  if (pageMark == false) {
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
void Book::CreatePage(std::string sBuffer, std::string& sConvert, 
    size_t page, bool mark) {
  //Add entry, or update entry in map of words/ pages (new page + relevance)
  for (auto it : func::extractWordsFromString(sBuffer)) {
    map_words_pages_[it.first].AddPage(page);
    map_words_pages_[it.first].AddRelevance(it.second);
  } 


  //Create new page
  std::ofstream write(path_ + "/intern/page" + std::to_string(page) + ".txt");
  write << func::returnToLower(sBuffer);
  write.close();

  if (mark == false)
    sConvert += "\n\n----- "+std::to_string(page)+" / 999 -----\n\n" + sBuffer;
}

/**
* @brief Find preview position for each word in map of words/pages.
*/
void Book::CreateMapPreview() {
  std::cout << "Create map Prev." << std::endl;
  for(auto it : map_words_pages_) 
    map_words_pages_[it.first].set_position(GetPreviewPosition(it.first));
}
    

/**
* @brief safe map of all words and pages to disc
*/
void Book::SafePages() {
  std::cout << "Saving pages" << std::endl;
  //Open file.
  std::ofstream write(path_ + "/intern/pages.txt");
  write << num_pages_ << "\n";

  //Iterate over map of words
  for (auto it : map_words_pages_) {
    //Write word in converted format (replace a by ä, é by e ...)
    std::string word = it.first;
    std::string sBuffer = func::convertStr(word) + ";";

    //Add all pages word occurs on
    for (size_t page : it.second.pages())
      sBuffer += std::to_string(page) + ",";

    //Add preview-position and relevance.
    sBuffer += ";" + std::to_string(map_words_pages_[it.first].relevance());
    sBuffer += ";" + std::to_string(map_words_pages_[it.first].position());
    //sBuffer += ";" + std::to_string(std::get<1>(map_words_pages_[it.first]));
    //sBuffer += ";" + std::to_string(std::get<2>(map_words_pages_[it.first])) 
      + "\n";

    write << sBuffer;
  }
  write.close();
}


/**
* @brief load words and pages on which word occurs into map
*/
void Book::LoadPages() {
  std::cout << key_ << "Loading pages." << std::endl;
  //Load map
  std::ifstream read(path_ + "/intern/pages.txt");
  std::string sBuffer;

  //Read number of pages, if first lind does not indicate pages, recreate book.
  getline(read, sBuffer);
  if (std::isdigit(sBuffer[0]) == false) {
    CreatePages();
    CreateMapPreview();
    SafePages();
    return;
  }
  num_pages_ = stoi(sBuffer);

  //Read words, pages, and relevance, preview-position
  while (!read.eof()) {
    //Read new line
    getline(read, sBuffer);
    if(sBuffer.length() < 2)
      continue;

    //Extract word (vec[0] = word, vec[1] = sBuffer
    std::vector<std::string> vec = func::split2(sBuffer, ";");

    //Extract pages and convert to size_t
    std::vector<size_t> pages;
    for (auto page : func::split2(vec[1], ",")) {
      //Check if page actually is number, then add to pages
      if (isdigit(page[0]))
        pages.push_back(std::stoi(page));
    }

    //Add word and pages to map
    map_words_pages_[vec[0]] = WordInfo(pages, std::stoi(vec[3]), std::stoi(vec[2]));
  }

  read.close();
}


// **** GET PAGES FUNCTIONS **** //


/**
* @brief GetPages calls FindPages (extracting all pages from given word) for each 
* word searched and removes duplicates and/ or pages, where not all words searched
* occur.
* @param[in] sInput (list of searched words as a string, separated by ' ' or + 
* @param[in] fuzzyness (boolean indicating whether fuzziness is set or not)
* @return map of pages, with vector of words on this page 
* (all the same if fuzziness==false)
*/
std::map<int, std::vector<std::string>>* Book::GetPages(std::string sInput, 
    bool fuzzyness) {
  //Initialize new map and return empty map in case the book has no ocr.
  auto* mapPages = new std::map<int, std::vector<std::string>>;
  if (has_ocr_ == false)
    return mapPages;

  //Do some parsing, as user can use ' ' or + to indicate searching for several words.
  std::replace(sInput.begin(), sInput.end(), ' ', '+');
  std::vector<std::string> vWords = func::split2(func::returnToLower(sInput), "+");

  //Create map of pages and found words for first word
  mapPages = FindPages(vWords[0], fuzzyness);

  //Iterate over all 1..n words create list of pages, and remove words not on same page.
  for (size_t i=1; i<vWords.size(); i++) {
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
std::map<int, std::vector<std::string>>* Book::FindPages(std::string sWord, 
    bool fuzzyness) {
  //Create empty list of pages
  auto* mapPages = new std::map<int, std::vector<std::string>>;

  //Normal-search
  if (fuzzyness == false) {
    //Check for different grammatical forms.
    if (found_grammatical_matches_.count(sWord) > 0) {
      //Obtain pages from each word.
      for (auto elem : found_grammatical_matches_[sWord]) {
        for (auto page : map_words_pages_.at(elem).pages())
            (*mapPages)[page].push_back(elem);
      }
    }
    //Obtains pages.
    else if (map_words_pages_.count(sWord) > 0) {
      for (auto page : map_words_pages_.at(sWord).pages())
        (*mapPages)[page].push_back(sWord);
    }
  }

  //Fuzziness-search 
  else {
    //Check for words in map of fuzzy matches.
    if (found_fuzzy_matches_.count(sWord) > 0) {
      //Obtain pages from each word found by fuzzy-search.
      for (auto elem : found_fuzzy_matches_[sWord]) {
        for (auto page : map_words_pages_.at(elem.first).pages())
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
void Book::RemovePages(std::map<int, std::vector<std::string>>* results1, 
    std::map<int, std::vector<std::string>>* results2) {
  //Iterate over results-1.
  for (auto it=results1->begin(); it!=results1->end();) {
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
bool Book::OnSamePage(std::vector<std::string> vWords, bool fuzzyness) {
  //Check if words occur only in metadata (Author, Title, Date)
  if (MetadataCmp(vWords, fuzzyness) == true)
    return true;

  //Get all pages, the first word is on
  std::vector<size_t> pages1 = PagesFromWord(vWords[0], fuzzyness);

  //If no pages or found, return false
  if (pages1.size() == 0) 
    return false;

  //Iterate over words 1..n
  for (size_t i=1; i<vWords.size(); i++) {
    //get all pages of word i.
    std::vector<size_t> pages2 = PagesFromWord(vWords[i], fuzzyness);

    //Return false if pages are empty
    if (pages2.size() == 0) 
      return false;

    //Check if all pages in pages-1 occur in pages-2.
    bool found=false;
    for (size_t j=0; j<pages1.size(); j++) {
      if(std::find(pages2.begin(), pages2.end(), pages1[j]) != pages2.end()) {
        found=true;
        break;
      }
    }
    if (found==false) 
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
bool Book::MetadataCmp(std::vector<std::string> vWords, bool fuzzyness) {
  bool inMetadata = true;
          
  //Iterate over all words and check for occurrence in metadata
  for (const auto& word : vWords) {
    std::string sTitle = metadata_.GetTitle(); func::convertToLower(sTitle);
    std::string sAuthor = metadata_.GetAuthor(); func::convertToLower(sAuthor);

    //If word occurs neither in author, title, or date, set inMetadata to false.
    if (sAuthor.find(word) == std::string::npos  
        && sTitle.find(word) == std::string::npos 
        && std::to_string(date_) != word) 
      inMetadata = false;

    //Check title and author again with fuzzy search.
    if (fuzzyness == true) {
      //Check title
      bool found = false;
      sTitle = func::convertStr(sTitle);
      for (auto it : func::extractWordsFromString(sTitle)) {
        if(fuzzy::fuzzy_cmp(word, it.first) <= 0.2)
          found = true;
      }

      //check author
      sAuthor = func::convertStr(sAuthor);
      for (auto it : func::extractWordsFromString(sAuthor)) {
        if(fuzzy::fuzzy_cmp(word, it.first) <= 0.2)
          found = true;
      }
      
      //if found with fuzzy-search, set check-variable to true.
      if (found == true)
        inMetadata = found;
    }

    if (inMetadata == false)
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
std::vector<size_t> Book::PagesFromWord(std::string sWord, bool fuzzyness)
{
  //Use set to automatically erase duplicates.
  std::set<size_t> pages;

  //Obtain pages with exact match
  if (map_words_pages_.count(sWord) > 0) {
    std::vector<size_t> foo = map_words_pages_.at(sWord).pages();
    pages.insert(foo.begin(), foo.end());
  }

  //Obtain pages from grammatical matches
  if (found_grammatical_matches_.count(sWord) > 0) {
    for (auto elem : found_grammatical_matches_[sWord]) {
      std::vector<size_t> foo = map_words_pages_.at(elem).pages();
      pages.insert(foo.begin(), foo.end());
    }
  }

  //Obtain pages from fuzzy match.
  if (found_fuzzy_matches_.count(sWord) > 0 && fuzzyness == true)  {
    for (auto elem : found_fuzzy_matches_[sWord]) {
      std::vector<size_t> foo = map_words_pages_.at(elem.first).pages();
      pages.insert(foo.begin(), foo.end());
    }
  }
  
  //convert set to vector.
  std::vector<size_t> vec;
  vec.assign(pages.begin(), pages.end());
  return vec;
}


// **** GET PREVIEW FUNCTIONS **** //

/**
 * Get a preview for each searched word. Try to highlight/ find words
 * in one preview. Only add a new preview if word could not be found in
 * the preview created so far.
 * @param input (user input, possibly containing several words)
 * @return one ore more previews ready formatted for output.
 */
std::string Book::GetPreview(std::string input) {
  //Get vector of search words.
  std::vector<std::string> words = func::split2(input, "+");
  //Get First preview
  std::string prev = GetOnePreview(words[0]);

  //Get previous for extra words, that have been searched for.
  for (size_t i=1; i<words.size(); i++) {
    //Try finding second word in current preview
    size_t pos = prev.find(words[i]);
    if (pos!=std::string::npos) {
      prev.replace(pos, prev.find(" ", pos+1)-pos, "<mark>"+words[i]+"</mark>");
    } 
    else {
      std::string new_prev = GetOnePreview(words[i]);
      if(new_prev != "No Preview.")
        prev += "\n" + new_prev;
    }
  }
  return prev;
}

/**
 * @brief get a preview of a single word. (~150 characters + highlighting)
 * @param sWord (searched word)
 * @return Preview.
 */
std::string Book::GetOnePreview(std::string word) {
  size_t pos = 0, page = 1000000;

  //Get Source string and position, if no source, then return "no preview".
  std::string preview = "";
  if (has_ocr_ == true) 
    preview = GetPreviewText(word, pos, page);
  else
    preview = GetPreviewTitle(word, pos);
  if (preview=="") return "No Preview.";

  //Highlight searched word.
  if (pos > 75) pos = 75;
  preview.replace(pos, preview.find(" ", pos+1)-pos, "<mark>"+word+"</mark>");

  //Delete or escape invalid characters if needed.
  EscapeDeleteInvalidChars(preview);

  //Append [...] front and back
  preview.insert(0, " \u2026"); 
  if (page!=1000000)
    return preview += " \u2026 (S. " + std::to_string(page) +")";
  return preview += " \u2026";
}

/**
 * @brief return page on which word occures, the position on the page and the
 * page number.
 * @param[in] word (searched word)
 * @param[in, out] pos (position on the page)
 * @param[in, out] page (page number the word was found on)
 * @return complete text of the page the word was found on.
 */
std::string Book::GetPreviewText(std::string& word, size_t& pos, size_t& page) {
  //Get match (full, gramatical, fuzzy)
  if (map_words_pages_.count(word) > 0)
    word = word;
  else if (found_grammatical_matches_.count(word) > 0) {
    word = found_grammatical_matches_[word].front();
    if (map_words_pages_.count(word) == 0)
      return "";
  }
  else if (found_fuzzy_matches_.count(word) > 0) 
    word = found_fuzzy_matches_[word].front().first;
  else 
    return "";

  //Get page number from map of words and pages.
  page = map_words_pages_[word].pages()[0];
  if (page == 1000000) 
    return "";

  //Find position, where word occurs.
  pos = map_words_pages_[word].position();

  //Calculate beginning and ende of 150 character preview surounding word.
  size_t front = 0, len = 150;
  if (pos > 75) front = pos - 75;

  //Generate substring of 150 characters, with search word in the center.
  std::ifstream file(path_ + "/intern/page" + std::to_string(page) + ".txt",
      std::ifstream::ate | std::ifstream::binary);
  std::string preview = "";
  if (file.is_open()) {
    size_t file_length = file.tellg();
    if(front+len >= file_length) len = file_length-front;
    file.seekg(front);
    preview.resize(len);
    file.read(&preview[0], len); 
    file.close();
  }

  return preview;
}

/**
 * Get the complete title and the position where the word was found.
 * Only used for books without ocr.
 * @param[in] word (searched word)
 * @param[in, out] pos (position, where in title word was found)
 * @return title.
 */
std::string Book::GetPreviewTitle(std::string& word, size_t& pos) {
  //Get title
  std::string sTitle = metadata_.GetTitle() + " ";
  func::convertToLower(sTitle);

  //Find position where searched word occures.
  pos = sTitle.find(word);

  //If word could not be found. Try to find it with fuzzy search. 
  if (pos == std::string::npos || sTitle == "" || pos > sTitle.length()) {
    for (auto it : func::split2(sTitle, " ")) {
      if (fuzzy::fuzzy_cmp(func::convertStr(it), word) < 0.2) {
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
* @param[in] word (best Match)
* @param[in] page (page on which match was found)
* @return preview for this book
*/
size_t Book::GetPreviewPosition(std::string word) {
  std::vector<size_t> pages = map_words_pages_[word].pages();
  for (size_t i=0; i<pages.size(); i++) {
    //Read ocr and kontent and find match
    std::ifstream read(path_ + "/intern/page" + std::to_string(pages[i]) 
        + ".txt", std::ios::in);
    std::string page((std::istreambuf_iterator<char>(read)), 
        std::istreambuf_iterator<char>());
    size_t pos = page.find(word);

    //Return "error" if not found
    if (pos != std::string::npos) return pos;
  }
  
  return 1000000; //Maybe change the default value some time?
}

/**
 * Delete brocken characters and escape invalid literals.
 * @param[in, out] str (string to escape)
 */
void Book::EscapeDeleteInvalidChars(std::string& str) {
  //Delete invalid chars front
  for(;;) {
    if((int)str.front() < 0)
      str.erase(0,1);
    else if((int)str.back() < 0)
      str.pop_back();
    else
      break;
  }

  //Check vor invalid literals and escape
  for(unsigned int i=str.length(); i>0; i--) {
    if(str[i-1] == '\"' || str[i-1] == '\'' || str[i-1] == '\\')
      str.erase(i-1, 1);
    if(str[i-1] == '<' && str[i] != 'm' && str[i] != '/')
      str.erase(i-1, 1);
  }
}

/**
 * Add a single new pages, generate by Teseract. 
 * Also save the page to disc and add it to complete ocr. 
 * @param[in] page_text (text on page)
 * @param[in] page (page number) 
 * @param[in] max_page (number of pages in book)
 */
void Book::AddPage(std::string page_text, std::string pagenumber, 
    std::string max_pagenumber) {

  //Write the single given page to disc
  std::ofstream write_page(path_ + "/intern/add" + pagenumber + ".txt");
  write_page << page_text;
  write_page.close();

  //Add given page to existing ocr.
  fs::path p = ocr_path();
  fs::exists(p);
  std::string new_ocr;

  //If path doesnt exist, simple set current page + pagemark as new_ocr. 
  if (fs::exists(p) == false) {
    std::cout << "create new ocr..." << std::endl;
    page_text.insert(0, "\n----- " + pagenumber + " / "+ max_pagenumber 
        + " -----\n"); 
    new_ocr = page_text;
  }

  //Load 
  else {
    //Load all pages to be added and sort by page-number.
    std::map<int, std::string> order_pages;
    std::cout << "Adding to existing ocr ... " << std::endl;
    for (auto& p : fs::directory_iterator(path_ + "/intern")) {
      std::string filename = p.path().filename().string();

      //Check for correct format, then add to ordered map of pages.
      if (filename.find("add") != std::string::npos) {
        std::string num = filename.substr(3, filename.length()-2-filename.
            find("."));
        order_pages[stoi(num)] = p.path().string();
      }
    }

    //Iterate over all (sorted) pages: load and add each to ocr
    for(auto it : order_pages) {
      //Add pagemark
      new_ocr.append("\n\n----- " + std::to_string(it.first) + " / " 
          + max_pagenumber+" -----\n");

      //Load and add to ocr
      std::ifstream new_page(it.second);
      std::string str((std::istreambuf_iterator<char>(new_page)), 
          std::istreambuf_iterator<char>());
      new_ocr.append(str);
    }
  }

  std::ofstream write_ocr(path_ + "/ocr.txt");
  write_ocr << new_ocr;
  write_ocr.close();
}

/**
 * Returns the 10 most relevant neighboors of a given word.
 * @param[in] word
 * @return vector of the 10 most relevant neighboors.
 */
std::string Book::GetMostRelevantNeighbors(std::string word, Dict& dict) {
  std::map<std::string, int> neighbors;
  if (map_words_pages_.count(word) == 0)
    return "";
  for (auto page : map_words_pages_[word].pages()) {
    //Get source string of complete page. 
    std::ifstream read(path_ + "/intern/page" + std::to_string(page) + ".txt", 
        std::ios::in);
    std::string source((std::istreambuf_iterator<char>(read)), 
        std::istreambuf_iterator<char>());
    size_t pos = source.find(word);
    if (pos == std::string::npos) continue;

    std::string part = "";
    while (true) {
      size_t front = source.find(".");
      if (front == std::string::npos || front > pos) front = 0;
      size_t end = source.find(".", pos);
      if (end == std::string::npos) end = pos+30; 
      if (end >= source.length()) end = source.length();
      part = source.substr(front, end-front);
      size_t pos = source.find(word, pos);
      if (pos == std::string::npos) break;
    }
    for (auto it : func::extractWordsFromString(part)) {
      if (dict.IsWordX(it.first, "f", {"ADJ", "SUB", "VER"}) == true)
        neighbors[it.first] += it.second*(it.second+1)/2;
    }
  }
  
  //Create compare function
	typedef std::function<bool(std::pair<std::string, int>, 
      std::pair<std::string, int>)> Comp;
  Comp comp_function = [](const auto& a, const auto& b) {
    if (a.second == b.second)
      return a.first > b.first;
    return a.second > b.second;
  };

  //Convert map to sorted set
	std::set<std::pair<std::string, int>, Comp> sorted(neighbors.begin(), 
      neighbors.end(), comp_function);

  //Convert set to vector
  size_t counter = 0;
  std::string result="";
  for (auto it : sorted) {
    result += it.first + "(" + std::to_string(it.second) + ")" + ", ";
    if(++counter == 10) break;
  }

  return result;
}
  
