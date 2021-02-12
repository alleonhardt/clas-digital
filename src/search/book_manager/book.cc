#include "book.h"
#include "book_manager/word_info.h"
#include "func.hpp"
#include "nlohmann/json.hpp"
#include "search_object.h"
#include "sorted_matches.h"
#include "tmp_word_info.h"
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>

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

std::unordered_map<std::string, SortedMatches>& Book::corpus_fuzzy_matches() {
  return corpus_fuzzy_matches_;
}

std::unordered_map<std::string, SortedMatches>& Book::metadata_fuzzy_matches() {
  return metadata_fuzzy_matches_;
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
  if (!std::filesystem::exists(path) || std::filesystem::is_empty(path) || reload_pages)
    CreateIndex();
  else
    LoadPages();
}

void Book::CreateIndex() {
  // Delete old index-directory and create new.
  fs::remove_all(path_ + "/intern");
  fs::create_directories(path_ + "/intern");

  std::map<std::string, TempWordInfo> temp_map_pages;
  SeperatePages(temp_map_pages);
  CreateMapPreview(temp_map_pages);
  GenerateBaseFormMap(temp_map_pages);
  SafePages();
}

void Book::SeperatePages(temp_index_map& temp_map_pages) {
  std::cout << "Creating map of words... \n";
  // Load ocr and create ne directory "intern" for this book.
  std::ifstream read(ocr_path());
  
  std::string buffer = "", cur_line = "";
  size_t page=1, num_pages_=0;
  while (!read.eof()) {
    getline(read, cur_line);
    
    // Add a page when new page is reached.
    if (func::checkPage(cur_line)) {
      // Add new page to map of pages and update values. Safe page to disc.
      CreatePage(temp_map_pages, buffer, page);
      
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
    CreatePage(temp_map_pages, buffer, page);

  read.close();
}

void Book::CreatePage(temp_index_map& temp_map_pages, std::string sBuffer, size_t page) {

  // Extract words and calculate number of found words.
  auto extracted_words = func::extractWordsFromString(sBuffer);
  size_t num_words_on_page;
  // Add page to word and increase relevance.
  for (auto it : extracted_words) {
    temp_map_pages[it.first].AddPage({page, num_words_on_page});
    temp_map_pages[it.first].IncreaseRelevance(it.second);
  } 
  // Save page to disc.
  func::WriteContentToDisc(path_ + "/intern/page" + std::to_string(page) + ".txt", sBuffer);
}

void Book::CreateMapPreview(temp_index_map& temp_map_pages) {
  std::cout << "Create map Prev." << std::endl;
  for(auto it : temp_map_pages) {
    size_t best_page = temp_map_pages[it.first].GetBestPage();
    temp_map_pages[it.first].set_preview_position(GetPreviewPosition(it.first, best_page));
    temp_map_pages[it.first].set_preview_page(best_page);
  }
}

size_t Book::GetPreviewPosition(std::string word, size_t best_page) {
  // Read ocr and content and find match.
  std::ifstream read(path_ + "/intern/page" + std::to_string(best_page) + ".txt", std::ios::in);
  std::string page((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());
  size_t pos = page.find(word);

  // Stop indexing and exit programm, when preview could not be found.
  if (pos == std::string::npos) {
    std::cout << "Preview not found! " << key_ << ", " << word << ", " << best_page << std::endl;
    std::exit(404);  
  }
  return pos;
}

void Book::GenerateBaseFormMap(temp_index_map& temp_map_pages) {
  // Convert all words in map, if this results in a duplicate, join word-infos.
  temp_index_map map_pages_handle_duplicates;
  for (auto it : temp_map_pages) {
    // Full conversion of word (to lower, non utf-8 removed).
    std::string cur_word = it.first;
    cur_word = func::convertStr(func::returnToLower(cur_word));
    map_pages_handle_duplicates[cur_word].Join(it.second); 
  }

  // Add base-form to map_words_pages_ and current word as one of it conjunctions.
  for (auto it : temp_map_pages) {
    // Get base-form of word (hunden -> hund)
    std::string cur_word = it.first;
    std::string base_form = dict_->GetBaseForm(it.first);
    // If the base-form was not found, set the current word as it's own base-form.
    if (base_form == "") 
      base_form = cur_word;

    // Create word info of current word:
    double relevance = static_cast<double>(it.second.relevance())/num_pages_;
    WordInfo word_info{cur_word, it.second.GetAllPages(), it.second.preview_position(),
        it.second.preview_page(), relevance};
    map_words_pages_[base_form].push_back(word_info);
  }
}

void Book::SafePages() {
  std::cout << "Saving pages" << std::endl;
  // Open file for writing and write number of pages as first value.
  nlohmann::json json_index_map;
  json_index_map["num_pages"] = num_pages_;
  json_index_map["base_forms"] = nlohmann::json::object();

  // Iterate over all base forms and create word-info-entries for it's conjunctions.
  for (auto base_form : map_words_pages_) {
    json_index_map[base_form.first] = nlohmann::json::array();
    // Iterate over conjunctions and create word-info-entries.
    for (auto word_info : base_form.second) {
      // Create word-info-entry.
      nlohmann::json entry = nlohmann::json::object();
      entry["word"] = word_info.word_;
      entry["pages"] = word_info.pages_;
      entry["preview_position"] = word_info.preview_position_;
      entry["preview_page"] = word_info.preview_page_;
      entry["relevance"] = word_info.relevance_;

      // Add entry to base-form.
      json_index_map[base_form.first].push_back(entry);
    }
  }

  // Write constructed json to disc.
  std::ofstream write(path_ + "/intern/pages.txt");
  write << json_index_map;
  write.close();
}

void Book::LoadPages() {
  std::cout << key_ << " Loading pages." << std::endl;
  // Load json with index-map from disc.
  nlohmann::json json_index_map;
  std::ifstream read(path_ + "/intern/pages.txt");
  read >> json_index_map;
  read.close();

  // Extract number of pages.
  num_pages_ = json_index_map["num_pages"];

  // Iterate over all base forms and create word-info-entries for it's conjunctions.
  for (auto base_form = json_index_map["base_forms"].begin(); 
      base_form < json_index_map["base_forms"].end(); base_form++) {
    // Iterate over conjunctions and create word-info-entries.
    for (auto word_info : base_form.value()) {
      // Create word-info-entry and add to base-form.
      map_words_pages_[base_form.key()].push_back({
          word_info["word"],
          word_info["pages"],
          word_info["preview_position"],
          word_info["preview_page"],
          word_info["relevance"]
        });
    }
  }
}


// **** GET PAGES FUNCTIONS **** //

std::map<int, std::vector<std::string>> Book::GetPages(SearchObject search_object) {

  std::vector<std::string> words = search_object.converted_words();
  if (words.size() == 0) 
    return std::map<int, std::vector<std::string>>();

  // Create map of pages and found words for first word.
  auto map_pages = FindPagesAndMatches(words[0], search_object.search_options().fuzzy_search());

  // Iterate over all 1..n words create list of pages, and remove words not on same page.
  for (size_t i=1; i<words.size(); i++) {
    // Get pages from word i and remove all pages, which don't occure in both results
    auto map_pages2 = FindPagesAndMatches(words[i], search_object.search_options().fuzzy_search());
    RemovePages(map_pages, map_pages2);
  }
  return map_pages;
}

std::map<int, std::vector<std::string>> Book::FindPagesAndMatches(std::string word, 
    bool fuzzy_search) {
  // Find matching base_forms.
  auto base_forms = FindBaseForms(word, fuzzy_search);

  // Find pages for all conjunctions corresponding to base-form.
  std::map<int, std::vector<std::string>> map_pages;
  for (auto base_form : base_forms) {
    for (auto conjunction  : map_words_pages_[base_form]) {
      for (auto page : conjunction.pages_)
        map_pages[page].push_back(conjunction.word_);
    }
  }
  return map_pages;
}

std::vector<size_t> Book::FindOnlyPages(std::string word, bool fuzzy_search) {
  // Find matching base_forms.
  auto base_forms = FindBaseForms(word, fuzzy_search);
  
  // Find pages for all conjunctions corresponding to base-form.
  std::vector<size_t> pages;
  for (auto base_form : base_forms) {
    for (auto conjunction  : map_words_pages_[base_form]) {
      for (auto page : conjunction.pages_)
        pages.push_back(page);
    }
  }
  return pages;
}

std::vector<std::string> Book::FindBaseForms(std::string word, bool fuzzy_search) {
  std::vector<std::string> base_forms;
  if (map_words_pages_.count(word) > 0) 
    base_forms.push_back(word);
  if (fuzzy_search && corpus_fuzzy_matches_.count(word) > 0) {
    for (auto match : corpus_fuzzy_matches_)
      base_forms.push_back(match.first);
  }
  if (base_forms.size() == 0)
    std::cout << "\x1B[31mNo base form found!! \033[0m\t\t" << word << std::endl;
  return base_forms;
}

void Book::RemovePages(
    std::map<int, std::vector<std::string>>& results1, 
    std::map<int, std::vector<std::string>>& results2) {
  // Iterate over results-1.
  for (auto it=results1.begin(); it!=results1.end();) {
    // Erase if, page does not exist in results-2.
    if (results2.count(it->first) == 0)
      it = results1.erase(it);
    // Insert words on page i in results-2 into words on page i in results-1.
    else {
      it->second.insert(it->second.end(), results2[it->first].begin(), results2[it->first].end());
      ++it;
    }
  }
}

bool Book::OnSamePage(std::vector<std::string> vWords, bool fuzzyness) {
  //Check if words occur only in metadata (Author, Title, Date)
  if (MetadataCmp(vWords, fuzzyness) == true)
    return true;
  // Get all pages, the first word is on and return false if no pages found
  auto pages1 = FindOnlyPages(vWords[0], fuzzyness);
  if (pages1.size() == 0) 
    return false;

  // Iterate over words 1..n.
  for (size_t i=1; i<vWords.size(); i++) {
    // Get all pages of word i.
    std::vector<size_t> pages2 = FindOnlyPages(vWords[i], fuzzyness);

    // Remove all pages from pages-1, which don't occur on pages-2
    pages1.erase( remove_if( begin(pages1), end(pages1),
          [&](auto x){ return find(begin(pages2),end(pages2), x)==end(pages2);}), end(pages1) );
    if (pages1.size() == 0)
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
  else if (corpus_fuzzy_matches_.count(word) > 0) 
    word = corpus_fuzzy_matches_[word].GetBestMatch();
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
