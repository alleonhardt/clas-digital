/**
 * @author fux
 */

#include <chrono>
#include <cstddef>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>

#include <httplib.h>

#include "book_manager/book.h"
#include "book_manager/book_manager.h"
#include "func.hpp"
#include "gramma.h"
#include "nlohmann/json.hpp"
#include "search/search_options.h"

using namespace httplib;

std::string GetReqParam(const Request&req, std::string key, std::string def="") {
  if (req.has_param(key.c_str()))
    return req.get_param_value(key.c_str());
  return def;
}

/**
 * Search in all entries in corpus and all metadata.
 * @param[in] req (request)
 * @param[in, out] resp (respsonse)
 * @param[in] manager (book manager, to do search and get map of all books.)
 */
void Search(const Request& req, Response& resp, const nlohmann::json& 
    zotero_pillars, BookManager& manager, Dict& dict) {
  
  // Get and process query (necessary value!):
  if (!req.has_param("q")) {
    resp.status = 400;
    return;
  }
  std::string query = GetReqParam(req, "q");
  query = func::convertStr(query); // replace some non-english characters.
  func::transform(query); // remove leading and trailing non-characters.
  std::replace(query.begin(), query.end(), ' ', '+');

  // Get searcing for relevant neighbors:
  bool relevant_neighbors = GetReqParam(req, "relevant_neighbors", "0") == "1";
  // Get fuzzyness:
  bool fuzzyness = GetReqParam(req, "fuzzyness", "0") == "1";
  // Get scope:
  std::string scope = GetReqParam(req, "scope", "all");
  // Get author:
  std::string author = GetReqParam(req, "author");
  // Get sorting type:
  std::string sort = GetReqParam(req, "sorting", "relevance");

  // Get pillars:
  std::vector<std::string> pillars;
  std::string str_pillars;
  if (req.has_param("pillars")) {
    str_pillars = req.get_param_value("pillars", 0);
    for (auto it : func::split2(str_pillars, ","))
        pillars.push_back(it);
  }
  else {
    for (auto& it : zotero_pillars) {
      str_pillars += it["key"].get<std::string>() + ";";
      pillars.push_back(it["key"]);
    }
  }

  // Get published after/ published before:
  int pubafter = 1700, pubbefore = 2049;
  try{ 
    pubafter = std::stoi(req.get_param_value("publicatedafter")); 
  } catch(...) {};
  try{ 
    pubbefore = std::stoi(req.get_param_value("publicatedbefore")); 
  } catch(...) {};

  // Get limit and start: results per page, and current page, user is on.
  int resultsperpage = 10;
  try{ 
    resultsperpage = std::stoi(req.get_param_value("limit", 0)); 
  } catch(...) {};
  int list_start = 0;
  try { 
    list_start = std::stoi(req.get_param_value("start")); 
  } catch(...) {};

  // Debug printing.
  std::cout << "Recieved search request!" << std::endl;
  std::cout << "Searching with query: " << query << 
    "; fuzzyness: " << fuzzyness <<
    "; scope: " << scope <<
    "; author: " << author <<
    "; publicated after: " << pubafter <<
    "; publicated before: " << pubbefore << 
    "; searched pillars: " << str_pillars << 
    "; vector pillar size: " << pillars.size() <<
    "; sorting with value: " << sort << " and max results per page: " 
    << resultsperpage << std::endl;

  // Construct search-options.
  SearchOptions options(query, fuzzyness, pillars, scope, author, pubafter,
    pubbefore, true, sort);

  // Start search.
  auto time_start = std::chrono::system_clock::now();
  auto result_list = manager.DoSearch(&options);

  // Construct response.
  std::cout << "Constructing response json." << std::endl;
  nlohmann::json search_response;

  if (result_list.size() == 0) {
    search_response["max_results"] = 0;
    search_response["time"] = 0;
  }
  else {
    size_t counter = 0;
    for (auto book : result_list) {
      // Create entry for each book in result.
      nlohmann::json entry;
      entry["scanId"] = book->key();
      entry["copyright"] = !book->metadata().GetPublic();
      entry["hasocr"] = book->HasContent();
      entry["description"] = book->GetAuthorDateScanned();
      entry["bibliography"] = book->metadata().GetMetadata("bib");

      std::string preview = book->GetPreview(query);
      if (relevant_neighbors == true) {
        std::string str = book->GetMostRelevantNeighbors(query, dict);
        if (str != "") preview += "<br>" + str;
      }
      entry["preview"] = preview;
      search_response["books"].push_back(std::move(entry)); 
      if (++counter == resultsperpage)
        break;
    }

    // Add number of results and elapsed time to response.
    search_response["max_results"] = result_list.size();
    auto elapsed_seconds = std::chrono::system_clock::now() - time_start;
    search_response["time"] = elapsed_seconds.count();
  }
  std::cout << "Finished constructing json response." << std::endl;
  resp.set_content(search_response.dump(), "application/json");
}

/**
 * Return all pages of a book + all matches on each page.
 * @param[in] req (request)
 * @param[in, out] resp (respsonse)
 * @param[in] manager (book manager, to do get book which to search in.)
 */
void Pages(const Request& req, Response& resp, BookManager& manager) {
  if (!req.has_param("scanId") || !req.has_param("query") || !req.has_param("fuzzyness")) {
    std::cout << "Invalid or missing query-parameter in search_in_book: " << std::endl; 
    resp.status = 400;
    return;
  }

  // Retrieve necessary query and scan-/ book-id.
  std::string scan_id = req.get_param_value("scanId"); 
  std::string query = req.get_param_value("query"); 
  func::convertToLower(query);
  query = func::convertStr(query);
  
  int fuzzyness=0;
  try {
    fuzzyness = stoi(req.get_param_value("fuzzyness"));
  } catch (std::exception& e) {
    std::cout << "Recieved fuzzyness with invalid value, using 0 instead.";
  }
  
  // Construct response.
  nlohmann::json json_response;
  json_response["is_fuzzy"] = fuzzyness == 1;

  std::cout << "Search in book with query: " << query 
    << " and fuzzyness: " << fuzzyness << std::endl;
    
  // Check if book exists.
  if (manager.map_of_books().count(scan_id) == 0) {
    std::cout << "Search in book: given book not found!" << std::endl;
    resp.status = 404;
  }

  // Get pages.
  auto book = manager.map_of_books()[scan_id];
  auto pages = book->GetPages(query, fuzzyness!=0);
  std::cout << "Got " << pages->size() << " pages for this book." << std::endl;
  if (pages->size() == 0) {
    resp.status = 200;
    resp.set_content(json_response.dump(), "application/json");
    return;
  }

  // Create hitlist of books.
  try {
    json_response["books"] = nlohmann::json::array();
    for (auto const &it : *pages) {
      nlohmann::json entry;
      entry["page"] = it.first;
      std::string matches_on_page = "";
      for (auto const& match : it.second) {
        if (matches_on_page != "")
          matches_on_page +=", ";
        matches_on_page += std::regex_replace(match, std::regex("\""), "\\\"");
      }
      entry["words"] = matches_on_page;
      json_response["books"].push_back(entry);
    }
  }
  catch (std::exception& e) {
    std::cout << "Search in book: internal server error: " << e.what() << std::endl;
    resp.status = 500;
    return;
  }

  resp.status = 200;
  resp.set_content(json_response.dump(), "application/json");
}

/**
 * Returns suggestions
 * @param[in] req (request)
 * @param[in, out] resp (respsonse)
 * @param[in] manager (book manager, to do search for suggestions)
 * @param[in] type (indication what kind of suggestions (author/ corpus))
 */
void Suggestions(const Request& req, Response& resp, BookManager& manager, 
    std::string type) {
  std::cout << "Suggestions: " << type << std::endl;
  // Retrieve suggestions from book manager.
  std::string query = GetReqParam(req, "q", "");
  if (query == "") {
    resp.status = 400;
    return;
  }
  std::cout << "Suggestions: Got query: " << query << std::endl;

  std::list<std::string> suggestions;
  try {
    suggestions = manager.GetSuggestions(query, type);
  } catch (std::exception& e) {
    std::cout << "Suggestion: internal server error: " << e.what() << std::endl;
    resp.status = 500;
    return;
  }

  // Convert list to json.
  nlohmann::json json_response = nlohmann::json::array();
  size_t counter=0;
  for (auto suggestion : suggestions)
    json_response.push_back(suggestion);

  // Send response.
  resp.set_content(json_response.dump(), "application/json");
  resp.status = 200;
}

int main() {
  std::cout << "Starting super fast, but partly sucking c++ search api...\n" << std::endl;
  
  // Create server.
  Server srv;

  // Load and parse config file.
  nlohmann::json config;
  std::ifstream read_config("server.config");
  read_config >> config;
  read_config.close();

  // Load dictionary.
  std::cout << "Loading dictionary at " << config["dictionary"] << std::endl;
  Dict dict(config["dictionary"]);
  std::cout << "done.\n\n";

  // Load corpus metadata from disc.
  std::cout << "Loading metadata at " << config["zotero_metadata"];
  std::ifstream read(config["zotero_metadata"], std::ios::in);
  // Check if metadata was found.
  if(!read) {
    std::cout << "]nNo metadata found! Server fails to load\n";
    return 1;
  }
  nlohmann::json metadata;
  read >> metadata;
  std::cout << "done.\n\n";

  // Load active pillars:
  std::cout << "Loading active collections...";
  nlohmann::json zotero_pillars = nlohmann::json::array();
  for (auto it : metadata["collections"]["data"])
    zotero_pillars.push_back(nlohmann::json({{"key", it["key"]}, {"name", it["data"]["name"]}}));
  std::cout << "done.\n\n";

  // Create book manager:
  std::cout << "initializing bookmanager..." << std::endl;
  BookManager manager(config["upload_points"], config["dictionary_old"]);
  manager.UpdateZotero(metadata["items"]["data"]);
  if (manager.Initialize())
    std::cout << "Initialization successful!\n\n"; 
  else
    std::cout << "Initialization failed!\n\n";
  
  // Specify port to run on.
  int start_port = std::stoi("4848");
  std::cout << "Starting on port: " << start_port << std::endl;

  // Add specific handlers via server-frame 
  srv.Get("/api/v2/search", [&](const Request& req, Response& resp) 
      { Search(req, resp, zotero_pillars, manager, dict); });
  srv.Get("/api/v2/search/pages", [&](const Request& req, Response& resp) 
      { Pages(req, resp, manager); });
  srv.Get("/api/v2/search/suggestions/corpus", [&](const Request& req, Response& resp)
      { Suggestions(req, resp, manager, "corpus"); });
  srv.Get("/api/v2/search/suggestions/author", [&](const Request& req, Response& resp)
      { Suggestions(req, resp, manager, "author"); });

  std::cout << "C++ Api server startup successfull!" << std::endl;
  srv.listen("0.0.0.0", start_port);
  return 0;
}
