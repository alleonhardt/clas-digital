#include "book.h"
#include "func.hpp"
#include "fuzzy.hpp"
#include "nlohmann/json.hpp"
#include "search_object.h"
#include "sorted_matches.h"
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

Dict* Book::dict_ = nullptr;
void Book::set_dict(Dict* dict) { Book::dict_ = dict; }

Book::Book () {}

Book::Book(nlohmann::json metadata) : metadata_(metadata) {
  // These are only pre-initialized and finally set by InitializeBook().
  path_ = ""; 
  has_ocr_ = false;
  has_images_ = false;
  num_pages_ = 0;

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

std::unordered_map<std::string, std::vector<WordInfo>>& Book::map_words_pages() { 
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
  std::cout << "Creating map of words for " << key_ << "... \n";
  // Load ocr and create ne directory "intern" for this book.
  std::ifstream read(ocr_path());
  
  std::string buffer = "", cur_line = "";
  size_t page=1; 
  num_pages_ = 0;
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

void Book::ConvertWords(temp_index_map& temp_map_pages) {
  // Convert all words in map, if this results in a duplicate, join word-infos.
  temp_index_map map_pages_handle_duplicates;
  for (auto it : temp_map_pages) {
    // Full conversion of word (to lower, non utf-8 removed).
    std::string cur_word = it.first;
    cur_word = func::convertStr(func::returnToLower(cur_word));
    map_pages_handle_duplicates[cur_word].Join(it.second); 
  }
  temp_map_pages = map_pages_handle_duplicates;
}

void Book::GenerateBaseFormMap(temp_index_map& temp_map_pages) {
  // Converts all words to lower case and removes non utf-8 characters 
  // also handles resulting dublicates.
  ConvertWords(temp_map_pages);

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
  std::cout << "Number of pages: " << num_pages_;
  json_index_map["base_forms"] = nlohmann::json::object();

  // Iterate over all base forms and create word-info-entries for it's conjunctions.
  for (auto base_form : map_words_pages_) {
    json_index_map["base_forms"][base_form.first] = nlohmann::json::array();
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
      json_index_map["base_forms"][base_form.first].push_back(entry);
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
      base_form != json_index_map["base_forms"].end(); base_form++) {
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
    for (auto match : corpus_fuzzy_matches_[word].GetAllMatches())
      base_forms.push_back(match);
  }
  if (base_forms.size() == 0)
    std::cout << "\x1B[31mNo base form found!! \033[0m\t\t" << word << std::endl;
  return base_forms;
}

WordInfo Book::FindBestWordInfo(std::string original_word, 
    std::string converted_word, bool fuzzy_search) {
  
  // Get all matching baseforms and take first, as automatically best.
  std::vector<std::string> base_forms = FindBaseForms(converted_word, fuzzy_search);
  if (base_forms.size() == 0) {
    std::cout << "\x1B[31mNo base form found!! \033[0m\t\t" << converted_word << std::endl;
    throw std::out_of_range("No baseform found!");
  }
  std::string base_form = base_forms[0];

  if (map_words_pages_.count(base_form) == 0) {
    std::cout << "\x1B[31mBaseform not in map of words!! \033[0m\t\t" << base_form << std::endl;
    throw std::out_of_range("Baseform not in map of words:" + base_form);
  }

  // Find best matching conjunction.
  double cur_best_score = 0.2;
  std::string cur_best_match = base_form;
  for (auto conjunction : map_words_pages_[base_form]) {
    double score = fuzzy::fuzzy_cmp(original_word, conjunction.word_);
    if (score < cur_best_score) {
      cur_best_score = score;
      cur_best_match = conjunction.word_;
    }
  }
  for (auto conjunction : map_words_pages_[base_form]) {
    if (conjunction.word_ == cur_best_match) 
      return conjunction;
  }
  return map_words_pages_[base_form].front();
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

// **** GET PREVIEW FUNCTIONS **** //

std::string Book::GetPreview(SearchObject& search_object, bool in_corpus) {
  // Get vector of search words and first preview.
  std::vector<std::string> words = search_object.words();
  std::vector<std::string> converted_words = search_object.converted_words();
  std::string preview = "";

  // Get previous for extra words, that have been searched for.
  for (size_t i=0; i<converted_words.size(); i++) {
    
    // Try finding second (converted) word in current preview.
    size_t pos = preview.find(converted_words[i]);
    if (pos == std::string::npos) 
      pos = preview.find(words[i]);

    // If found, simply hightlight this word also.
    if (pos!=std::string::npos) {
      func::HighlightWordByPos(preview, pos, "<mark>", "</mark>");
    } 
    // Otherwise find a new preview for next word.
    else {
      std::string new_prev = GetOnePreview(words[i], converted_words[i], 
          search_object.search_options().fuzzy_search(), in_corpus);
      if(new_prev != "No Preview.")
        preview += "\n" + new_prev;
    }
  }
  return preview;
}

std::string Book::GetOnePreview(std::string original_word, std::string converted_word, 
    bool fuzzy_search, bool in_corpus) {
  size_t pos = -1, page = 1000000;
  std::string prev_str = "";

  if (in_corpus) {
    WordInfo word_info = FindBestWordInfo(original_word, converted_word, fuzzy_search);
    pos = word_info.preview_position_;
    page = word_info.preview_page_;
    prev_str = func::LoadStringFromDisc(path_ + "/intern/page" + std::to_string(page) + ".txt");
    if (prev_str == "") {
      std::cout << "\x1B[31mPreview-Page n ot loaded!! \033[0m\t\t" << std::endl;
      throw std::out_of_range("Page not found!");
    }
  }
  else {
    prev_str = GetPreviewMetadata(original_word, converted_word, fuzzy_search, pos);
  }

  if (prev_str == "") 
    return "No Preview";

  // Highlight searched word, only if exact position was found.
  func::HighlightWordByPos(prev_str, pos, "<mark>", "</mark>");

  // Trim string (text, position to center, min threschold, result-length)
  func::TrimString(prev_str, pos, 150);

  // Delete or escape invalid characters if needed.
  func::EscapeDeleteInvalidChars(prev_str);

  // Append [...] front and back.
  prev_str = "\u2026" + prev_str + "\u2026";
  return (page != 1000000) ? prev_str + " S. " + std::to_string(page) : prev_str;
}

std::string Book::GetPreviewMetadata(std::string original_word,
      std::string converted_word, bool fuzzy_search, size_t& pos) {
  // Find position where searched word occurs.
  pos = quick_title_.find(converted_word);
  if (pos != std::string::npos) 
    return quick_title_;

  // If word could not be found. Try to find it with fuzzy search. 
  for (auto it : quick_title_words_) {
    if (fuzzy::fuzzy_cmp(it.first, converted_word) < 0.2) {
      pos = quick_title_.find(it.first);
      if(pos == std::string::npos && pos > quick_title_.length())
        pos = -1; 
      return quick_title_;
    }
  }
  return "";
}

void Book::AddPage(std::string page_text, std::string pagenumber, 
    std::string max_pagenumber) {
  // Write the single given page to disc.
  func::WriteContentToDisc(path_ + "/intern/add" + pagenumber + ".txt", page_text);

  // Create new ocr
  std::string new_ocr;
  // If path doesn't exist, simply set current page + pagemark as new_ocr. 
  if (fs::exists(ocr_path()) == false) {
    std::cout << "create new ocr..." << std::endl;
    page_text.insert(0, "\n----- " + pagenumber + " / "+ max_pagenumber + " -----\n"); 
    new_ocr = page_text;
  }

  else {
    // Load all pages to be added and sort by page-number.
    std::map<int, std::string> order_pages;
    std::cout << "Adding to existing ocr ... " << std::endl;
    for (auto& p : fs::directory_iterator(path_ + "/intern")) {
      std::string filename = p.path().filename().string();

      // Check for correct format, then add to ordered map of pages.
      if (filename.find("add") != std::string::npos) {
        std::string num = filename.substr(3, filename.length()-2-filename.find("."));
        order_pages[stoi(num)] = p.path().string();
      }
    }

    // Iterate over all (sorted) pages: load and add each to ocr
    for (auto it : order_pages) {
      // Add pagemark, then load new page from disc and add to ocr.
      new_ocr.append("\n\n----- " + std::to_string(it.first) + " / " + max_pagenumber+" -----\n");
      new_ocr.append(func::LoadStringFromDisc(it.second));
    }
  }

  // Write new ocr to disc.
  func::WriteContentToDisc(path_ + "/ocr.txt", new_ocr);
}
