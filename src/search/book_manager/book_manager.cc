#include "book_manager.h"
#include "book_manager/book.h"
#include "search/search.h"
#include <ostream>
#include <utility>
#include <vector>

BookManager::BookManager(std::vector<std::string> mount_points, std::string dictionary_low) 
  : upload_points_(mount_points){
  std::string sBuffer;
  std::ifstream read(dictionary_low);
  
  while(getline(read, sBuffer)) {
    std::vector<std::string> vec = func::split2(sBuffer, ":");
    std::vector<std::string> vec2 = func::split2(vec[1], ",");
    for(size_t i=0; i<vec2.size(); i++) {
      dict_[vec[0]].insert(vec2[i]);
      dict_[vec2[i]].insert(vec[0]);
    }
  }
  std::cout << "Created dictionary. Size: " << dict_.size() << "\n";
}

std::unordered_map<std::string, Book*>& BookManager::map_of_books() {
  return map_books_;
}

BookManager::MAPWORDS& BookManager::map_of_authors() {
  return map_words_authors_;
}

std::map<std::string, std::vector<std::string>>& BookManager::map_unique_authors() {
  return map_unique_authors_;
}

void BookManager::WriteListofBooksWithBSB() {    
  std::string untracked_books = "Currently untracked books:\n";
  std::string inBSB_noOcr = "Buecher mit bsb-link, aber ohne OCR: \n";
  std::string gibtEsBeiBSB_noOCR = "Buecher mit tag \"gibtEsBeiBSB\" aber ohne ocr: \n";
  std::string gibtEsBeiBSB_OCR = "Buecher mit tag \"gibtEsBeiBSB\" aber mit ocr: \n";
  std::string BSBDownLoadFertig_noOCR = "Buecher mit tag \"BSBDownLoadFertig\" aber ohne ocr: \n";
  std::string tierwissen_noOCR = "Buecher in \"Geschichte des Tierwissens\" aber ohne ocr: \n";

  for (auto it : map_books_) {
    std::string info = it.second->key() + ": " + it.second->metadata().GetShow2() + "\n";
    if (it.second->has_ocr() == false) {
      //Check if book has a bsb-link (archiveLocation)
      if (it.second->metadata().GetMetadata("archiveLocation", "data") != "")
        inBSB_noOcr += info;
      
      //Check if book has tag: "GibtEsBeiBSB"
      if (it.second->metadata().HasTag("GibtEsBeiBSB") == true)
        gibtEsBeiBSB_noOCR += info;

      //Check if book has tag: "BSBDownLoadFertig"
      if (it.second->metadata().HasTag("BSBDownLoadFertig") == true)
        BSBDownLoadFertig_noOCR += info;

      //Check if book is in collection: "Geschichte des Tierwissens"
      if (func::in("RFWJC42V", it.second->metadata().GetCollections()) == true)
        tierwissen_noOCR += info;
    }
    else if (it.second->metadata().HasTag("GibtEsBeiBSB") == true)
      gibtEsBeiBSB_OCR += info;
  }

  //Search for untracked books
  std::vector<std::filesystem::path> vec;
  for (const auto& entry : std::filesystem::directory_iterator("web/books")) {
    std::string fileNameStr = entry.path().filename().string();
    if (entry.is_directory() && map_books_.count(fileNameStr) == 0) {
      std::ifstream readJson(entry.path().string() + "/info.json");
      if (!readJson) 
        continue;
      nlohmann::json j;
      readJson >> j;
      if (j.count("data") == 0 || j["data"].count("creators") == 0 
          || j["data"]["creators"][0].size() == 0) {
        continue;
      }
      std::string sName = j["data"]["creators"][0].value("lastName", "unkown name");
      std::string sTitle = j["data"].value("title", "unkown title");
      untracked_books += fileNameStr + " - " + sName + ", \"" + sTitle + "\"\n";
      readJson.close(); 
      vec.push_back(entry); 
    }
  }

  //Move old zotero files to archive: "zotero_old"
  if (std::filesystem::exists("web/books/zotero_old") == false)
    std::filesystem::create_directories("web/books/zotero_old");
  for (const auto& it : vec) {
    std::string path=it.string().substr(0, 9)+"/zotero_old"+it.string().substr(9);
    try {
      std::filesystem::rename(it, path);
    }
    catch (std::filesystem::filesystem_error& e) {
      std::cout << e.what() << std::endl;
    }
  }

  //Save books with bsb-link but o ocr
  std::ofstream writeBSB_NoOCR("inBSB_noOcr.txt");
  writeBSB_NoOCR << inBSB_noOcr;
  writeBSB_NoOCR.close();

  //Save books with tag GibtEsBeiBSB but without ocr
  std::ofstream writeGIBT_NoOCR("gibtEsBeiBSB_noOCR.txt");
  writeGIBT_NoOCR << gibtEsBeiBSB_noOCR;
  writeGIBT_NoOCR.close();

  //Save books with tag GibtEsBeiBSB but with ocr
  std::ofstream writeGIBT_OCR("gibtEsBeiBSB_OCR.txt");
  writeGIBT_OCR << gibtEsBeiBSB_OCR;
  writeGIBT_OCR.close();

  //Save books with tag BSBDownLoadFertig but with ocr
  std::ofstream writeFERTIG_noOCR("BSBDownLoadFertig_noOCR.txt");
  writeFERTIG_noOCR << BSBDownLoadFertig_noOCR;
  writeFERTIG_noOCR.close();

  //Save books in collection "Geschichte des Tierwissens": RFWJC42V, without ocr
  std::ofstream writeTierwissen_noOCR("tierwissen_noOCR.txt");
  writeTierwissen_noOCR << tierwissen_noOCR;
  writeTierwissen_noOCR.close();

  //Write all untracked books to seperate files
  if(std::filesystem::exists("untracked_books.txt"))
    std::filesystem::rename("untracked_books.txt", "web/books/zotero_old/untracked_books.txt");
  if(vec.size() > 0) {
    std::ofstream writeUntracked("untracked_books.txt");
    writeUntracked << untracked_books;
    writeUntracked.close();
  }
}

bool BookManager::Initialize() {
  std::cout << "BookManager: Starting initializing..." << std::endl;

  std::cout << "BookManager: Extracting books." << std::endl;
  //Go though all books and create book
  for (auto upload_point : upload_points_) {
    for (const auto& p : std::filesystem::directory_iterator(upload_point)) {
      std::string filename = p.path().stem().string();
      if (map_books_.count(filename) > 0)
        AddBook(p.path(), filename);
    }
  }

  //Create map of all words + and of all words in all titles
  std::cout << "BookManager: Creating map of books." << std::endl;
  CreateMapWords();
  CreateMapWordsTitle();
  CreateMapWordsAuthor();
  std::cout << "Map words:   " << map_words_.size() << "\n";
  std::cout << "Map title:   " << map_words_title_.size() << "\n";
  std::cout << "Map authors: " << map_words_authors_.size() << "\n";

  // WriteListofBooksWithBSB();
  return true;
}

void BookManager::UpdateZotero(nlohmann::json j_items) {
  //Iterate over all items in json
  for (auto &it : j_items) {
    // already exists: update metadata.
    if (map_books_.count(it["key"]) > 0)
      map_books_[it["key"]]->metadata().set_json(it);

    // does not exits: create new book and add to map of all books.
    else
      map_books_[it["key"]] = new Book(it);

    // If book's json does not contain the most relevant information, delete again
    if(map_books_[it["key"]]->metadata().CheckJsonSet() == false) {
      delete map_books_[it["key"]];
      map_books_.erase(it["key"]);
    }
  }
}

void BookManager::AddBook(std::string path, std::string sKey) {
  if(!std::filesystem::exists(path))
    return;
  map_books_[sKey]->InitializeBook(path);
}
    

std::list<Book*> BookManager::DoSearch(SearchOptions* searchOpts) {
    std::vector<std::string> sWords = func::split2(searchOpts->getSearchedWord(), "+");
  //Start first search:
  Search search(searchOpts, sWords[0]);
  auto* results = search.search(map_words_, map_words_title_, map_words_authors_, 
      map_books_, dict_);

  // Do search for every other word.
  for (size_t i=1; i<sWords.size(); i++) {
    if(sWords[i].length() == 0)
      continue;

    Search search2(searchOpts, sWords[i]);
    auto* results2 = search2.search(map_words_, map_words_title_, 
        map_words_authors_, map_books_, dict_);

    for (auto it=results->begin(); it!=results->end();) {
      if(results2->count(it->first) == 0)
        it = results->erase(it);
      else if(map_books_[it->first]->OnSamePage(sWords, searchOpts->getFuzzyness())==false)
        it = results->erase(it);
      else
        ++it;
    }
  }
  //Sort results results and convert to list of books.
  std::list<Book*> sorted_search_results;
  for (auto it : SortMapByValue(results, searchOpts->getFilterResults()))
    sorted_search_results.push_back(map_books_[it.first]); 
  return sorted_search_results;
}

BookManager::sorted_set BookManager::SortMapByValue(
    std::map<std::string, double>* unordered_results, int sorting) {
  if (unordered_results->size() == 0) 
    return sorted_set();
  else if (unordered_results->size() == 1)
    return {std::make_pair(unordered_results->begin()->first, unordered_results->begin()->second)};

	// Defining a lambda function to compare two pairs. It will compare two pairs using second field.
	Comp compFunctor;
  if (sorting == 0) {
    compFunctor = [](const auto &a,const auto &b) {
        if(a.second == b.second) 
          return a.first > b.first;
        return a.second > b.second; };
  }
  else if (sorting == 1) {
    compFunctor = [this](const auto &elem1,const auto &elem2) {
        int date1 = map_books_[elem1.first]->date();
        int date2 = map_books_[elem2.first]->date();
        if(date1==date2) 
          return elem1.first < elem2.first;
        return date1 < date2; };
  }
  else {
    compFunctor = [this](const auto &elem1,const auto &elem2) {
        auto &bk1 = map_books_[elem1.first];
        auto &bk2 = map_books_[elem2.first];
        std::string s1= bk1->author();
        std::string s2= bk2->author();
        if(s1==s2)
          return elem1.first < elem2.first;
        return s1 < s2; };
  }

  //Sort by defined sort logic
	sorted_set sorted_results(unordered_results->begin(), unordered_results->end(), compFunctor);
  return sorted_results;
}


void BookManager::CreateMapWords() {
  // Iterate over all books. Add words to list and book to list of books with this word.
  for (auto it=map_books_.begin(); it!=map_books_.end(); it++) {
    // Check whether book has ocr.
    if(it->second->has_ocr() == false)
      continue;
    //Iterate over all words in book and add word to map and book to list with score.
    for(auto yt : it->second->map_words_pages()) {
      map_words_[yt.first][it->first] = static_cast<double>(it->second
          ->map_words_pages()[yt.first].relevance())/it->second->num_pages();
    }
  }

  CreateListWords(map_words_, list_words_);
} 

void BookManager::CreateMapWordsTitle() {
  // Iterate over all books. Add words from title to map.
  for (auto it=map_books_.begin(); it!=map_books_.end(); it++) {
    // Get map of words of current book.
    std::string sTitle = it->second->metadata().GetTitle();
    sTitle = func::convertStr(sTitle);
    std::map<std::string, int> mapWords = func::extractWordsFromString(sTitle);
    
    //Iterate over all words in this book. Check whether word already exists in list off all words.
    for (auto yt=mapWords.begin(); yt!=mapWords.end(); yt++)
      map_words_title_[yt->first][it->first] = 0.1;
    // Add author names and year.
    map_words_title_[std::to_string(it->second->date())][it->first] = 0.1;
  }
}

void BookManager::CreateMapWordsAuthor() {
  //Iterate over all books. Add words to list (if the word does not already exist 
  // in map of all words) or add book to list of books (if word already exists).
  for (auto it=map_books_.begin(); it!=map_books_.end(); it++) {
    for (auto author : it->second->metadata().GetAuthorsKeys()) {
      if (!it->second->metadata().IsAuthorEditor(author["creator_type"]))
        continue;
      map_words_authors_[func::returnToLower(author["lastname"])][it->first] = 0.1;
      map_unique_authors_[author["key"]].push_back(it->first);
    }
  }
  CreateListWords(map_words_authors_, list_authors_);
}

void BookManager::CreateListWords(MAPWORDS& mapWords, sortedList& listWords) {
  std::map<std::string, size_t> mapRel;
  for (auto it = mapWords.begin(); it!=mapWords.end(); it++) {
    mapRel[it->first] = it->second.size();
  }

  typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;

  Comp compFunctor = [](const auto &a, const auto &b) {
      if (a.second == b.second) 
        return a.first > b.first;
      return a.second > b.second; };

  std::set<std::pair<std::string, size_t>, Comp> sorted(mapRel.begin(), mapRel.end(), compFunctor);

  for (auto elem : sorted)
    listWords.push_back({elem.first, elem.second});
}


std::list<std::string> BookManager::GetSuggestions(std::string word, std::string scope) {
  if(scope=="corpus") 
    return GetSuggestions(word, list_words_);
  if(scope=="author") 
    return GetSuggestions(word, list_authors_);
  return std::list<std::string>();
}

std::list<std::string> BookManager::GetSuggestions(std::string word, sortedList& list_words) {
  std::cout << "GetSuggestions" << std::endl;
  func::convertToLower(word);
  word = func::convertStr(word);
  std::map<std::string, double>* suggs = new std::map<std::string, double>;
  size_t counter=0;
  for (auto it=list_words.begin(); it!=list_words.end() && counter < 10; it++) {
    double value = fuzzy::fuzzy_cmp(it->first, word);
    if(value > 0 && value <= 0.2) {
      (*suggs)[it->first] = value*(-1);
      counter++;
    }
  }
  std::list<std::string> sorted_search_results;
  for (auto it : SortMapByValue(suggs, 0)) 
    sorted_search_results.push_back(it.first); 
  delete suggs;
  return sorted_search_results;
}
