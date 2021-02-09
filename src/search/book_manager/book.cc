#include "book.h"
#include "func.hpp"
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

Book::Book () {}

Book::Book(nlohmann::json metadata) : metadata_(metadata) {
  // These are only pre-initialized and finally set by InitializeBook().
  path_ = ""; 
  has_ocr_ = false;
  has_images_ = false;

  // Set Metadata.
  key_ = metadata["key"];
  author_ = (metadata_.GetAuthor().size() == 0) ? "No Author" : metadata_.GetAuthor();
  author_date_ = author_;
  date_ = metadata_.GetDate();
  collections_ = metadata_.GetCollections();

  // Fast-access-members.
  author_date_ += (date_ != -1) ? ", " + std::to_string(date_) : author_date_ += ".";
  quick_author_ = metadata_.GetAuthor(); // what about multiple authors.
  func::convertToLower(quick_author_); 
  std::string converted_author = func::convertStr(quick_author_); // check if this is needed.
  quick_authors_words_ = func::extractWordsFromString(converted_author);
  quick_title_ = metadata_.GetTitle();
  func::convertToLower(quick_author_);  // check if this is needed.
  std::string converted_title = func::convertStr(quick_title_);
  quick_title_words_ = func::extractWordsFromString(converted_title);
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

std::unordered_map<std::string, WordInfo>& Book::map_words_pages() { 
  return map_words_pages_;
}

std::unordered_map<std::string, std::list<std::pair<std::string, double>>>& 
Book::found_fuzzy_matches() {
  return found_fuzzy_matches_;
}

std::unordered_map<std::string, std::list<std::string>>& Book::found_grammatical_matches() {
  return found_grammatical_matches_;
}

bool Book::HasContent() const { 
  return has_images_ || has_ocr_; 
}

std::string Book::GetAuthorDateScanned() {
  if(has_ocr_ == true)
    return author_date_;
  return author_date_ + "<span style='color:orange;font-size:80%;margin-left:1rem;'>"
    "Book is not yet scanned, sorry for that.</span>";
}    

// **** SETTER **** //

void Book::SetPath(std::string path) { 
  path_ = path; 
}


// **** CREATE BOOK AND MAPS (PAGES, RELEVANCE, PREVIEW) **** // 

void Book::InitializeBook(std::string path, bool reload_pages) {
  // Set absolute path.
  path_ = path;

  //Check if book has images
  for (auto& p: std::filesystem::directory_iterator(path)) {
    if(p.path().extension() == ".jpg" || p.path().extension() == ".bmp")
      has_images_ = true; 
  }

  // if books has ocr, pre-process ocr and create map of worlds/ pages.
  if (std::filesystem::exists(ocr_path()))
    InitializePreProcessing(reload_pages);
}

void Book::InitializePreProcessing(bool reload_pages) {
  has_ocr_ = true;

  //Write json to disc.
  func::WriteContentToDisc(path_ + "/info.json", metadata_.GetJson());

  // Check whether list of words_pages already exists, if yes load, otherwise, create
  std::string path = path_ + "/intern/pages.txt";
  if (!std::filesystem::exists(path) || std::filesystem::is_empty(path) || reload_pages) {
    CreatePages();
    CreateMapPreview(); // find (first) preview.
    ConvertKeys(); // convert word to only english!
    SafePages();
  }
  else
    LoadPages();
}

void Book::CreatePages() {
  std::cout << "Creating map of words... \n";
  // Load ocr and create ne directory "intern" for this book.
  std::ifstream read(ocr_path());
  fs::remove_all(path_ + "/intern");
  fs::create_directories(path_ + "/intern");

  std::string buffer = "", cur_line = "";
  size_t page=1, num_pages_=0;
  while (!read.eof()) {
    getline(read, cur_line);
    
    // Add a page when new page is reached.
    if (func::checkPage(cur_line)) {
      // Add new page to map of pages and update values. Safe page to disc.
      CreatePage(buffer, page);
      
      // Parse new page-number from line, reset buffer and increate page_counter.
      page = stoi(cur_line.substr(6, cur_line.find("/")-7));
      buffer = "";
      num_pages_++;
    }

    // Append line to buffer if page end is not reached.
    else
      buffer += " " + cur_line + "\n";
  }
  // If there is "something left", safe as new page.
  if (buffer.length() !=0)
    CreatePage(buffer, page);

  read.close();
}

void Book::CreatePage(std::string sBuffer, size_t page) {
  // Add entry, or update entry in map of words/ pages (new page + relevance)
  for (auto it : func::extractWordsFromString(sBuffer)) {
    map_words_pages_[it.first].AddPage(page);
    map_words_pages_[it.first].AddRelevance(it.second);
  } 
  // Create new page
  func::WriteContentToDisc(path_ + "/intern/page" + std::to_string(page) + ".txt", 
      func::returnToLower(sBuffer));
}

void Book::CreateMapPreview() {
  std::cout << "Create map Prev." << std::endl;
  for(auto it : map_words_pages_)
    map_words_pages_[it.first].set_position(GetPreviewPosition(it.first));
}

void Book::ConvertKeys() {
  std::unordered_map<std::string, WordInfo> map_words_pages_new;
  for (auto it : map_words_pages_) {
    std::string new_key = it.first;
    map_words_pages_new[func::convertStr(new_key)] = map_words_pages_[it.first];
  }
  map_words_pages_ = map_words_pages_new;
}

void Book::SafePages() {
  std::cout << "Saving pages" << std::endl;
  // Open file for writing and write number of pages as first value.
  std::ofstream write(path_ + "/intern/pages.txt");
  write << num_pages_ << "\n";

  // Iterate over map of words.
  for (auto it : map_words_pages_) {
    // Write word in converted format (replace a by ä, é by e ...).
    std::string buffer = it.first + ";";

    // Add all pages the word occurs on.
    for (size_t page : it.second.pages())
      buffer += std::to_string(page) + ",";

    // Add preview-position and relevance.
    buffer += ";" + std::to_string(map_words_pages_[it.first].relevance())
            + ";" + std::to_string(map_words_pages_[it.first].position()) + "\n";

    write << buffer;
  }
  write.close();
}

void Book::LoadPages() {
  std::cout << key_ << " Loading pages." << std::endl;
  // Load map.
  std::ifstream read(path_ + "/intern/pages.txt");
  std::string buffer = "";

  // Read number of pages, if first line does not indicate pages, recreate book.
  getline(read, buffer);
  if (std::isdigit(buffer[0]) == false) {
    CreatePages();
    CreateMapPreview();
    SafePages();
    return;
  }
  num_pages_ = stoi(buffer);

  // Read words, pages, and relevance, preview-position.
  while (!read.eof()) {
    getline(read, buffer);
    if(buffer.length() < 2) continue;

    // Extract word (vec[0] = word, vec[1] = sBuffer.
    std::vector<std::string> vec = func::split2(buffer, ";");

    // Extract pages and convert to size_t.
    std::vector<size_t> pages;
    for (auto page : func::split2(vec[1], ",")) {
      // Check if page actually is number, then add to pages.
      if (isdigit(page[0]))
        pages.push_back(std::stoi(page));
    }

    // Add word and pages to map.
    map_words_pages_[vec[0]] = WordInfo(pages, std::stoi(vec[3]), std::stoi(vec[2]));
  }
  read.close();
}


// **** GET PAGES FUNCTIONS **** //

std::map<int, std::vector<std::string>>* Book::GetPages(std::string input, 
    bool fuzzyness) {
  // Initialize new map and return empty map in case the book has no ocr.
  auto* map_pages = new std::map<int, std::vector<std::string>>;
  if (has_ocr_ == false)
    return map_pages;

  // Do some parsing, as user can use ' ' or + to indicate searching for several words.
  std::replace(input.begin(), input.end(), ' ', '+');
  std::vector<std::string> vWords = func::split2(func::returnToLower(input), "+");

  // Create map of pages and found words for first word.
  map_pages = FindPages(vWords[0], fuzzyness);

  // Iterate over all 1..n words create list of pages, and remove words not on same page.
  for (size_t i=1; i<vWords.size(); i++) {
    // Get pages from word i.
    auto* mapPages2 = FindPages(vWords[i], fuzzyness);

    // Remove all elements from mapPages, which do not exist in results2. 
    RemovePages(map_pages, mapPages2);
    delete mapPages2;
  }
  return map_pages;
}

std::map<int, std::vector<std::string>>* Book::FindPages(std::string sWord, 
    bool fuzzyness) {
  // Create empty list of pages
  auto* map_pages = new std::map<int, std::vector<std::string>>;

  // Normal-search:
  if (fuzzyness == false) {
    // Check for different grammatical forms.
    if (found_grammatical_matches_.count(sWord) > 0) {
      // Obtain pages from each word.
      for (auto elem : found_grammatical_matches_[sWord]) {
        for (auto page : map_words_pages_.at(elem).pages())
          (*map_pages)[page].push_back(elem);
      }
    }
    // Obtains pages.
    else if (map_words_pages_.count(sWord) > 0) {
      for (auto page : map_words_pages_.at(sWord).pages())
        (*map_pages)[page].push_back(sWord);
    }
  }

  // Fuzzy-search:
  else {
    // Check for words in map of fuzzy matches.
    if (found_fuzzy_matches_.count(sWord) > 0) {
      // Obtain pages from each word found by fuzzy-search.
      for (auto elem : found_fuzzy_matches_[sWord]) {
        for (auto page : map_words_pages_.at(elem.first).pages())
          (*map_pages)[page].push_back(elem.first);
      }
    }
  }
  return map_pages;
}

void Book::RemovePages(std::map<int, std::vector<std::string>>* results1, 
    std::map<int, std::vector<std::string>>* results2) {
  // Iterate over results-1.
  for (auto it=results1->begin(); it!=results1->end();) {
    // Erase if, page does not exist in results-2.
    if (results2->count(it->first) == 0)
      it = results1->erase(it);
    // Insert words on page i in results-2 into words on page i in results-1.
    else {
      std::vector<std::string> vec = (*results2)[it->first];
      it->second.insert(it->second.end(), vec.begin(), vec.end());
      ++it;
    }
  }
}

bool Book::OnSamePage(std::vector<std::string> vWords, bool fuzzyness) {
  //Check if words occur only in metadata (Author, Title, Date)
  if (MetadataCmp(vWords, fuzzyness) == true)
    return true;

  // Get all pages, the first word is on.
  std::vector<size_t> pages1 = PagesFromWord(vWords[0], fuzzyness);

  // If no pages or found, return false.
  if (pages1.size() == 0) 
    return false;

  // Iterate over words 1..n.
  for (size_t i=1; i<vWords.size(); i++) {
    // Get all pages of word i.
    std::vector<size_t> pages2 = PagesFromWord(vWords[i], fuzzyness);

    // Return false if pages are empty.
    if (pages2.size() == 0) 
      return false;

    // Check if all pages in pages-1 occur in pages-2.
    bool found=false;
    for (size_t j=0; j<pages1.size(); j++) {
      if(std::find(pages2.begin(), pages2.end(), pages1[j]) != pages2.end()) {
        found=true;
        break;
      }
    }
    if (!found) 
      return false;
  }
  
  return true;
}

bool Book::MetadataCmp(std::vector<std::string> words, bool fuzzyness) {
  bool in_metadata = true;
          
  // Iterate over all words and check for occurrence in metadata.
  for (const auto& word : words) {

    // If word occurs neither in author, title, or date, set inMetadata to false.
    if (quick_author_.find(word) == std::string::npos  
        && quick_title_.find(word) == std::string::npos 
        && std::to_string(date_) != word) 
      in_metadata = false;

    // Check title and author again with fuzzy search.
    if (fuzzyness == true) {
      // Check title:
      bool found = false;
      for (auto it : quick_title_words_) {
        if(fuzzy::fuzzy_cmp(word, it.first) <= 0.2)
          found = true;
      }

      // Check author:
      for (auto it : quick_authors_words_) {
        if(fuzzy::fuzzy_cmp(word, it.first) <= 0.2)
          found = true;
      }
      
      // If found with fuzzy-search, set check-variable to true.
      if (found == true)
        in_metadata = found;
    }

    if (in_metadata == false)
      return false;
  }
  return in_metadata;
}

std::vector<size_t> Book::PagesFromWord(std::string word, bool fuzzyness) {
  // Use set to automatically erase duplicates.
  std::set<size_t> pages;

  // Obtain pages with exact match.
  if (map_words_pages_.count(word) > 0) {
    std::vector<size_t> foo = map_words_pages_.at(word).pages();
    pages.insert(foo.begin(), foo.end());
  }

  // Obtain pages from grammatical matches.
  if (found_grammatical_matches_.count(word) > 0) {
    for (auto elem : found_grammatical_matches_[word]) {
      std::vector<size_t> foo = map_words_pages_.at(elem).pages();
      pages.insert(foo.begin(), foo.end());
    }
  }

  // Obtain pages from fuzzy match.
  if (found_fuzzy_matches_.count(word) > 0 && fuzzyness == true)  {
    for (auto elem : found_fuzzy_matches_[word]) {
      std::vector<size_t> foo = map_words_pages_.at(elem.first).pages();
      pages.insert(foo.begin(), foo.end());
    }
  }
  
  // Convert set to vector.
  std::vector<size_t> vec(pages.begin(), pages.end());
  return vec;
}


// **** GET PREVIEW FUNCTIONS **** //

std::string Book::GetPreview(std::string input) {
  // Get vector of search words and first preview.
  std::vector<std::string> words = func::split2(input, "+");
  std::string prev = GetOnePreview(words[0]);

  // Get previous for extra words, that have been searched for.
  for (size_t i=1; i<words.size(); i++) {
    // Try finding second word in current preview.
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

std::string Book::GetOnePreview(std::string word) {
  size_t pos = -1, page = 1000000;

  // Get Source string and position, if no source, then return "no preview".
  std::string preview = (has_ocr()) ? GetPreviewText(word, pos, page) : GetPreviewTitle(word, pos);
  
  // If no preview was found, return "No Preview"
  if (preview == "") 
    return "No Preview.";

  // Highlight searched word, only if exact position was found.
  if (pos != -1) {
    if (pos > 75) 
      pos = 75;
    preview.replace(pos, preview.find(" ", pos+1)-pos, "<mark>"+word+"</mark>");
  }

  // Delete or escape invalid characters if needed.
  EscapeDeleteInvalidChars(preview);

  // Append [...] front and back.
  preview.insert(0, " \u2026"); 
  if (page!=1000000)
    return preview += " \u2026 (S. " + std::to_string(page) +")";
  return preview += " \u2026";
}

std::string Book::GetPreviewText(std::string& word, size_t& pos, size_t& page) {
  // Get match (full, grammatical, fuzzy).
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

  // Get page number from map of words and pages.
  page = map_words_pages_[word].pages()[0];
  if (page == 1000000) 
    return "";

  // Find position, where word occurs.
  pos = map_words_pages_[word].position();

  // Calculate beginning and ende of 150 character preview surounding word.
  size_t front = 0, len = 150;
  if (pos > 75) front = pos - 75;

  // Generate substring of 150 characters, with search word in the center.
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

std::string Book::GetPreviewTitle(std::string& word, size_t& pos) {
  // Find position where searched word occurs.
  pos = quick_title_.find(word);
  if (pos != std::string::npos) 
    return quick_title_;

  // If word could not be found. Try to find it with fuzzy search. 
  for (auto it : quick_title_words_) {
    if (fuzzy::fuzzy_cmp(it.first, word) < 0.2) {
      pos = quick_title_.find(it.first);
      if(pos == std::string::npos && pos > quick_title_.length())
        pos = -1; 
      return quick_title_;
    }
  }
  return "";
}


size_t Book::GetPreviewPosition(std::string word) {
  std::vector<size_t> pages = map_words_pages_[word].pages();
  for (size_t i=0; i<pages.size(); i++) {
    // Read ocr and content and find match.
    std::ifstream read(path_ + "/intern/page" + std::to_string(pages[i]) 
        + ".txt", std::ios::in);
    std::string page((std::istreambuf_iterator<char>(read)), 
        std::istreambuf_iterator<char>());
    size_t pos = page.find(word);

    // Return "error" if not found.
    if (pos != std::string::npos) 
      return pos;
  }
  return 1000000; //Maybe change the default value some time?
}

void Book::EscapeDeleteInvalidChars(std::string& str) {
  // Delete invalid chars front.
  for (;;) {
    if ((int)str.front() < 0)
      str.erase(0,1);
    else if ((int)str.back() < 0)
      str.pop_back();
    else
      break;
  }

  //Check vor invalid literals and escape
  for (unsigned int i=str.length(); i>0; i--) {
    if (str[i-1] == '\"' || str[i-1] == '\'' || str[i-1] == '\\')
      str.erase(i-1, 1);
    if (str[i-1] == '<' && str[i] != 'm' && str[i] != '/')
      str.erase(i-1, 1);
  }
}

void Book::AddPage(std::string page_text, std::string pagenumber, 
    std::string max_pagenumber) {
  // Write the single given page to disc.
  std::ofstream write_page(path_ + "/intern/add" + pagenumber + ".txt");
  write_page << page_text;
  write_page.close();

  // Add given page to existing ocr.
  fs::path p = ocr_path();
  fs::exists(p);
  std::string new_ocr;

  // If path doesnt exist, simple set current page + pagemark as new_ocr. 
  if (fs::exists(p) == false) {
    std::cout << "create new ocr..." << std::endl;
    page_text.insert(0, "\n----- " + pagenumber + " / "+ max_pagenumber 
        + " -----\n"); 
    new_ocr = page_text;
  }

  // Load.
  else {
    // Load all pages to be added and sort by page-number.
    std::map<int, std::string> order_pages;
    std::cout << "Adding to existing ocr ... " << std::endl;
    for (auto& p : fs::directory_iterator(path_ + "/intern")) {
      std::string filename = p.path().filename().string();

      // Check for correct format, then add to ordered map of pages.
      if (filename.find("add") != std::string::npos) {
        std::string num = filename.substr(3, filename.length()-2-filename.
            find("."));
        order_pages[stoi(num)] = p.path().string();
      }
    }

    // Iterate over all (sorted) pages: load and add each to ocr
    for (auto it : order_pages) {
      // Add pagemark.
      new_ocr.append("\n\n----- " + std::to_string(it.first) + " / " 
          + max_pagenumber+" -----\n");

      // Load and add to ocr.
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

std::string Book::GetMostRelevantNeighbors(std::string word, Dict& dict) {
  std::map<std::string, int> neighbors;
  if (map_words_pages_.count(word) == 0)
    return "";
  for (auto page : map_words_pages_[word].pages()) {
    // Get source string of complete page. 
    std::ifstream read(path_ + "/intern/page" + std::to_string(page) + ".txt", std::ios::in);
    std::string source((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());
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
  
  // Create compare function.
	typedef std::function<bool(std::pair<std::string, int>, std::pair<std::string, int>)> Comp;
  Comp comp_function = [](const auto& a, const auto& b) {
    if (a.second == b.second)
      return a.first > b.first;
    return a.second > b.second;
  };

  // Convert map to sorted set.
	std::set<std::pair<std::string, int>, Comp> sorted(neighbors.begin(), 
      neighbors.end(), comp_function);

  // Convert set to vector.
  size_t counter = 0;
  std::string result="";
  for (auto it : sorted) {
    result += it.first + "(" + std::to_string(it.second) + ")" + ", ";
    if (++counter == 10) break;
  }
  return result;
}
