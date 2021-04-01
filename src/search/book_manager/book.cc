#include "book.h"
#include "func.hpp"
#include "fuzzy.hpp"
#include "nlohmann/json.hpp"
#include "search_object.h"
#include "sorted_matches.h"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

// Initialize static variables.
Dict* Book::dict_ = nullptr;
std::map<short, std::pair<std::string, double>> Book::metadata_tag_reference_ 
  = std::map<short, std::pair<std::string, double>>();
std::map<std::string, short> Book::reverted_tag_reference_ = std::map<std::string, short>();

Book::Book () {}

Book::Book(std::map<short, std::string> metadata) : metadata_(metadata) {
  // These are only pre-initialized and finally set by InitializeBook().
  path_ = ""; 
  has_ocr_ = false;
  has_images_ = false;
  num_pages_ = 0;

  // Get needed tags from metadata.
  key_ = GetFromMetadata("key");
  collections_ = func::split2(GetFromMetadata("collections"), ";");
  
  // date:
  date_ = -1;
  if (GetFromMetadata("date") != "") {
    int date = -1; 
    try {
      date_ = stoi(GetFromMetadata("date"));
    } catch (std::exception& e) {
      std::cout << "Failed to convert date: " << GetFromMetadata("date") << e.what() << std::endl;
      throw "Failed to convert date: " + GetFromMetadata("date") + e.what();
    }
  }

  CreateMetadataIndex();
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

int Book::date() {
  return date_;
}

std::vector<std::string> Book::collections() { 
  return collections_; 
}

std::unordered_map<std::string, std::vector<WordInfo>>& Book::words_on_pages() { 
  return words_on_pages_;
}

std::unordered_map<std::string, std::vector<WordInfo>>& Book::words_in_tags() { 
  return words_in_tags_;
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

bool Book::IsPublic() const {
  std::time_t ttime = time(0);
  tm *local_time = localtime(&ttime);

  //Check if rights are set in metadata.
  if (GetFromMetadata("rights") == "CLASfrei")
    return true;

  //Check for year books is published in. 
  if (date_ == -1 || date_ >= local_time->tm_year+1800)
    return false;
  return true;
}

std::string Book::GetFromMetadata(std::string tag) const {
  std::string str = metadata_.at(reverted_tag_reference_.at(tag));
  return str;
}

size_t Book::GetLength() const {
  return words_in_tags_.size() + words_on_pages_.size();
}
// **** SETTER **** //

void Book::SetPath(std::string path) { 
  path_ = path; 
}

void Book::UpdateMetadata(std::map<short, std::string> metadata) {
  metadata_ = metadata;
}

void Book::set_dict(Dict* dict) { 
  Book::dict_ = dict; 
}

void Book::set_metadata_tag_reference(std::map<short, std::pair<std::string, double>> ref) {
  metadata_tag_reference_ = ref;
}

void Book::set_reverted_tag_reference(std::map<std::string, short> ref) {
  reverted_tag_reference_= ref;
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

  // Check whether list of words_pages already exists, if yes load, otherwise, create
  std::string path = path_ + "/intern/pages.txt";
  if (!std::filesystem::exists(path) || std::filesystem::is_empty(path) || reload_pages)
    CreateCorpusIndex();
  else
    LoadPages();
}

void Book::CreateCorpusIndex() {
  // Delete old index-directory and create new.
  fs::remove_all(path_ + "/intern");
  fs::create_directories(path_ + "/intern");

  std::map<std::string, TempWordInfo> temp_map_pages;
  SeperatePages(temp_map_pages);
  CreateMapPreview(temp_map_pages);
  GenerateBaseFormMap(temp_map_pages, words_on_pages_);
  SafePages();
}

void Book::CreateMetadataIndex() {

  std::cout << "CreateMetadataIndex()" << std::endl;
  std::map<std::string, TempWordInfo> temp_map_words_in_tags;

  // For each metadata entry (tag), extract words and add to map of words in tags. 
  // As we are using the same structur as for words in corpus, keep in mind,
  // that pages or "locations", here being the tags, in which this word is
  // found.
  for (auto it : metadata_) {
    // Skip tag, has relevance 0, skip this tag.
    if (metadata_tag_reference_[it.first].second == 0)
      continue;
    auto extracted_words = func::extractWordsFromString(it.second);
    for (auto word_info : extracted_words) {
      temp_map_words_in_tags[word_info.first].AddPage({it.first, metadata_tag_reference_[it.first].second});
      temp_map_words_in_tags[word_info.first].IncreaseRawCount(metadata_tag_reference_[it.first].second);
    }
  }

  // Find preview position for each word. 
  std::cout << "Create map Prev. (metadata)" << std::endl;
  for(auto it : temp_map_words_in_tags) {
    std::string cur_word = it.first;

    // Get tag with the highest relevance.
    size_t best_tag = temp_map_words_in_tags[cur_word].GetBestPage();
    
    // Find position of word in this tag.
    size_t pos = metadata_[best_tag].find(it.first);
    
    // Stop indexing and exit programm, when preview could not be found.
    if (pos == std::string::npos) {
      std::cout << "Preview not found (metadata)! " << key_ << ", " << cur_word << ", " << best_tag 
        << ": " << metadata_[best_tag] << std::endl;
      std::exit(404);  
    }
    temp_map_words_in_tags[it.first].set_preview_position(pos);
    temp_map_words_in_tags[it.first].set_preview_page(best_tag);
  }

  // Convert word (lowercase, non-utf-8) and handle duplicates (join word-info-data)
  ConvertWords(temp_map_words_in_tags);

  // GenerateBaseFormMap for metadata
  GenerateBaseFormMap(temp_map_words_in_tags, words_in_tags_);
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
    temp_map_pages[it.first].IncreaseRawCount(it.second);
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

void Book::GenerateBaseFormMap(temp_index_map& temp_map_pages,
      std::unordered_map<std::string, std::vector<WordInfo>>& index_map) {

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
    double term_frequency = static_cast<double>(it.second.raw_count())/temp_map_pages.size();
    WordInfo word_info{cur_word, it.second.GetAllPages(), it.second.preview_position(),
        it.second.preview_page(), term_frequency};
    index_map[base_form].push_back(word_info);
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
  for (auto base_form : words_on_pages_) {
    json_index_map["base_forms"][base_form.first] = nlohmann::json::array();
    // Iterate over conjunctions and create word-info-entries.
    for (auto word_info : base_form.second) {
      // Create word-info-entry.
      nlohmann::json entry = nlohmann::json::object();
      entry["word"] = word_info.word_;
      entry["pages"] = word_info.pages_;
      entry["preview_position"] = word_info.preview_position_;
      entry["preview_page"] = word_info.preview_page_;
      entry["relevance"] = word_info.term_frequency_;

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
      words_on_pages_[base_form.key()].push_back({
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
  auto base_forms = FindBaseForms(word, fuzzy_search, words_on_pages_, corpus_fuzzy_matches_);

  // Find pages for all conjunctions corresponding to base-form.
  std::map<int, std::vector<std::string>> map_pages;
  for (auto base_form : base_forms) {
    for (auto conjunction  : words_on_pages_[base_form]) {
      for (auto page : conjunction.pages_)
        map_pages[page].push_back(conjunction.word_);
    }
  }
  return map_pages;
}

std::vector<size_t> Book::FindOnlyPages(std::string word, bool fuzzy_search) {
  // Find matching base_forms.
  auto base_forms = FindBaseForms(word, fuzzy_search, words_on_pages_, corpus_fuzzy_matches_);
  
  // Find pages for all conjunctions corresponding to base-form.
  std::vector<size_t> pages;
  for (auto base_form : base_forms) {
    for (auto conjunction  : words_on_pages_[base_form]) {
      for (auto page : conjunction.pages_)
        pages.push_back(page);
    }
  }
  return pages;
}

std::vector<std::string> Book::FindBaseForms(std::string word, bool fuzzy_search,
    const index_map_type& index_map, const fuzzy_result_type& found_fuzzy_matches) const {
  // Generate epty list of base_forms.
  std::vector<std::string> base_forms;

  // Check if word exists in index-map.
  if (index_map.count(word) > 0) 
    base_forms.push_back(word);

  // If not, check fuzzy matches and return all matching fuzzy results.
  if (fuzzy_search && found_fuzzy_matches.count(word) > 0) {
    for (auto match : found_fuzzy_matches.at(word).GetAllMatches())
      base_forms.push_back(match);
  }
  if (base_forms.size() == 0)
    std::cout << key_ << "\x1B[31mNo base form found (on same page)!! \033[0m\t\t" << word << std::endl;
  return base_forms;
}

WordInfo Book::FindBestWordInfo(std::string original_word, std::string converted_word, 
    bool fuzzy_search, const index_map_type& index_map, const fuzzy_result_type& found_fuzzy_matches) const {
  // Get all matching baseforms and take first, as automatically best.
  std::vector<std::string> base_forms = FindBaseForms(converted_word, fuzzy_search, index_map, found_fuzzy_matches);
  if (base_forms.size() == 0) {
    std::cout << "\x1B[31mNo base form found (preview)!! \033[0m\t\t" << converted_word << std::endl;
    throw std::out_of_range("No baseform found!");
  }
  std::string base_form = base_forms[0];

  if (index_map.count(base_form) == 0) {
    std::cout << "\x1B[31mBaseform not in map of words!! \033[0m\t\t" << base_form << std::endl;
    throw std::out_of_range("Baseform not in map of words:" + base_form);
  }

  // Find best matching conjunction by comparing all matches with original searced word.
  double cur_best_score = 0.2;
  std::string cur_best_match = base_form;
  // Calculate difference between each foudn fuzzy match and the original word
  // and identify best match.
  for (auto conjunction : index_map.at(base_form)) {
    double score = fuzzy::fuzzy_cmp(original_word, conjunction.word_);
    if (score < cur_best_score) {
      cur_best_score = score;
      cur_best_match = conjunction.word_;
    }
  }

  // Now we now the best match, we need to find it again. 
  for (auto conjunction : index_map.at(base_form)) {
    if (conjunction.word_ == cur_best_match) 
      return conjunction;
  }

  // if for some reason, nothing was found return the first conjunction.
  return index_map.at(base_form).front();
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

bool Book::OnSamePage(std::vector<std::string> words, bool fuzzyness) {
  // Get all pages, the first word is on and return false if no pages found
  auto pages1 = FindOnlyPages(words[0], fuzzyness);
  if (pages1.size() == 0) 
    return false;

  // Iterate over words 1..n.
  for (size_t i=1; i<words.size(); i++) {
    // Get all pages of word i.
    std::vector<size_t> pages2 = FindOnlyPages(words[i], fuzzyness);

    // Remove all pages from pages-1, which don't occur on pages-2
    pages1.erase( remove_if( begin(pages1), end(pages1),
          [&](auto x){ return find(begin(pages2), end(pages2), x)==end(pages2);}), end(pages1) );
    if (pages1.size() == 0)
      return false;
  }
  return true;
}

// **** GET PREVIEW FUNCTIONS **** //

std::string Book::GetPreview(const std::vector<SearchedWordObject>& words, bool fuzzy_search) {
  // Get vector of search words and first preview.
  std::string preview = "";

  // Get previous for extra words, that have been searched for.
  for (const auto& word : words) {
    
    // Try finding original word in current preview.
    size_t pos = preview.find(word.orginal_);
    // If not found, try finding converted word in current preview
    if (pos == std::string::npos) 
      pos = preview.find(word.converted_);

    // If found, simply hightlight this word also.
    if (pos!=std::string::npos) {
      func::HighlightWordByPos(preview, pos, "<mark>", "</mark>");
    } 
    // Otherwise find a new preview for next word.
    else {
      std::string new_prev = GetOnePreview(word.orginal_, word.converted_, word.scope_, fuzzy_search);
      if(new_prev != "No Preview.")
        preview += "\n" + new_prev;
    }
  }
  return preview;
}

std::string Book::GetOnePreview(std::string orginal, std::string converted, short scope, bool fuzzy_search) {
  // Pre initialize found position, location and preview string.
  size_t pos = -1, locaction = 0;
  std::string prev_str = "";

  // Find best word-info with help form found fuzzy matches (depending on found in corpus/ metadata).
  if (scope & 1) {
    WordInfo word_info = FindBestWordInfo(orginal, converted, fuzzy_search, words_on_pages_, corpus_fuzzy_matches_);
    pos = word_info.preview_position_;
    locaction = word_info.preview_page_;
    // Load page on which preview was found from disc.
    prev_str = func::LoadStringFromDisc(path_ + "/intern/page" + std::to_string(locaction) + ".txt");
  }
  else {
    WordInfo word_info = FindBestWordInfo(orginal, converted, fuzzy_search, words_in_tags_, metadata_fuzzy_matches_);
    pos = word_info.preview_position_;
    locaction = word_info.preview_page_;
    // Get tag-content from metadata.
    prev_str = metadata_[locaction];
  }

  // Check that preview was found. For testing reason, throw if not found (to
  // make 100% sure, that we find potential mistakes. THIS SHOULD BE CHANGED FOR
  // DEPLOYMENT! Mistakes can allways happen!
  if (prev_str == "") {
    std::cout << "\x1B[31mPreview-location not loaded!! \033[0m\t\t" << std::endl;
    throw std::out_of_range("Location not found!");
  }

  // Highlight searched word, only if exact position was found.
  func::HighlightWordByPos(prev_str, pos, "<mark>", "</mark>");

  // Trim string (text, position to center, min threschold, result-length)
  func::TrimString(prev_str, pos, 150);

  // Delete or escape invalid characters if needed.
  func::EscapeDeleteInvalidChars(prev_str);

  // Append [...] front and back.
  prev_str = "\u2026" + prev_str + "\u2026";

  // If found in corpus, append page number at the end of preview.
  if (scope & 1) 
    prev_str += " P. " + std::to_string(locaction);
  // Otherwise append tag name a the beginning of the preview.
  else {
   prev_str = metadata_tag_reference_[locaction].first + ": " + prev_str;
   prev_str.front() = std::toupper(prev_str.front());
  }
  return prev_str;
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
