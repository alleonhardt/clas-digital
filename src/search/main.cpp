/**
 * @author fux
 */

#include <chrono>
#include <iostream>
#include <string>

#include <httplib.h>

#include "book_manager/CBookManager.hpp"

using namespace httplib;

/**
 * Load a page (html/ css/ javascript) from disc and return as string
 * @param[in] path to file
 * @return complete file as string
 * Load the login
 */
std::string GetPage(std::string file) {
  //Read loginpage and send
  std::string path = "web/"+file;
  std::ifstream read(path);
  std::string login_page( (std::istreambuf_iterator<char>(read) ),
                           std::istreambuf_iterator<char>()     );

  //Delete file-end marker
  login_page.pop_back();
  return login_page;
}

/**
 * Search in all entries in corpus. And all metadata.
 * @param[in] req (request)
 * @param[in, out] resp (respsonse)
 * @param[in] manager (book manager, to do search and get map of all books.)
 */
void Search(const Request& req, Response& resp, const nlohmann::json& 
    zotero_pillars, CBookManager& manager) {
  
  try {
    //Get query (necessary value!)
    if (!req.has_param("q")) return;
    std::string query = req.get_param_value("q", 0);

    //get fuzzyness
    bool fuzzyness = false;
    if (req.has_param("fuzzyness")) 
      fuzzyness = std::stoi(req.get_param_value("fuzzyness", 0)) != 0;

    //get pillars
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

    //Get scope
    std::string scope = "all";
    if (req.has_param("scope")) 
      scope = req.get_param_value("scope", 0);

    //Get author
    std::string author = "";
    if (req.has_param("author"))
      author = req.get_param_value("author", 0);

    //Get published after/ published before.
    int pubafter = 1700, pubbefore = 2049;
    try{ pubafter = std::stoi(req.get_param_value("publicatedafter")); } 
    catch(...) {};
    try{ pubbefore = std::stoi(req.get_param_value("publicatedbefore")); } 
    catch(...) {};

    //Get sorting type
    std::string sort = "relevance";
    if (req.has_param("sorting"))
      sort = req.get_param_value("sorting", 0);

    //get limit and start: results per page, and current page, user is on.
    int resultsperpage = 10;
    try{ resultsperpage = std::stoi(req.get_param_value("limit", 0)); }
    catch(...) {};
    int list_start = 0;
    try { list_start = std::stoi(req.get_param_value("start")); }
    catch(...) {};

    //Debug printing.
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

    //Construct search-options
    CSearchOptions options(query, fuzzyness, pillars, scope, author, pubafter,
      pubbefore, true, sort);


    //Start search
    auto time_start = std::chrono::system_clock::now();
    auto result_list = manager.search(&options);

    //Construct response
    std::cout << "Constructing response json." << std::endl;
    auto all_books = manager.getMapOfBooks();
    nlohmann::json search_response;

    if (result_list->size() == 0) {
      search_response["max_results"] = 0;
      search_response["time"] = 0;
    }
    else {
      //get iterator and advance to beginning of "list_start".
      auto it = result_list->begin();
      std::advance(it, list_start);

      size_t counter = 0;
      for (; it!=result_list->end(); it++) {
        //Create entry for each book in result.
        auto &book = all_books[*it];
        nlohmann::json entry;
        entry["scanId"] = book->get_key();
        entry["copyright"] = !book->get_metadata().GetPublic();
        entry["hasocr"] = book->hasContent();
        entry["description"] = book->getAuthorDateScanned();
        entry["bibliography"] = book->get_metadata().GetMetadata("bib");
        entry["preview"] = book->getPreview(query);
        search_response["books"].push_back(std::move(entry)); 
        if (++counter == resultsperpage)
          break;
      }

      //Add number of results and elapsed time to response.
      search_response["max_results"] = result_list->size();
      std::chrono::duration<double> elapsed_seconds = 
        std::chrono::system_clock::now() - time_start;
      search_response["time"] = elapsed_seconds.count();
    }
    std::cout << "Finished constructing json response." << std::endl;

    resp.set_content(search_response.dump(), "application/json");
  }

  catch (std::exception &e) {
    std::cout << "Caught exception in search_all_books: " << e.what() << "\n";
    // TODO (fux): investigate what kind of response is necessary!!
    resp.status = 400;
    resp.set_content("<html><head></head><body><h1>Corrupted search request!"
        "</h1></body></html>", "text/html");
  }
}

/**
 * Return all pages of a book + all matches on each page.
 * @param[in] req (request)
 * @param[in, out] resp (respsonse)
 * @param[in] manager (book manager, to do get book which to search in.)
 */
void Pages(const Request& req, Response& resp, CBookManager& manager) {
  try {
    //Retrieve necessary date from url
    std::string scanId = req.get_param_value("scanId"); 

    std::string query = req.get_param_value("query"); 
    func::convertToLower(query);
    query = func::convertStr(query);
    
    int fuzzyness = stoi(req.get_param_value("fuzzyness"));

    //Get pages
    std::cout << "Search in book with query : " << query
      << " and fuzzyness: " << fuzzyness << std::endl;

    auto book = manager.getMapOfBooks()[scanId];
    auto pages = book->getPages(query, fuzzyness!=0);
    std::cout << "Got " << pages->size() << " pages for this book." << std::endl;

    //Construct response
    nlohmann::json json_response;
    if (fuzzyness == 0)
      json_response["is_fuzzy"] = false;

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
      json_response.push_back(std::move(entry));
    }

    resp.set_content(json_response, "application/json");
  }

  catch (std::exception &e) {
    std::cout << "Caught exception in search_in_book: " << e.what() << "\n"; 
    // TODO (fux): investigate what kind of response is necessary!!
  }
}

/**
 * Returns suggestions
 * @param[in] req (request)
 * @param[in, out] resp (respsonse)
 * @param[in] manager (book manager, to do search for suggestions)
 * @param[in] type (indication what kind of suggestions (author/ corpus))
 */
void Suggestions(const Request& req, Response& resp, CBookManager& manager, 
    std::string type) {
  try {
    //Retrieve suggestions from book manager.
    std::string query = req.get_param_value("q");
    std::list<std::string>* suggestions = manager.getSuggestions(query, type);

    //Convert list to json.
    nlohmann::json json_response;
    for (auto it=suggestions->begin(); it!=suggestions->end(); it++)
      json_response.push_back(*it);
    delete suggestions;

    //Send response.
    resp.set_content(json_response.dump(), "application/json");
  }

  catch(std::exception &e) {
    std::cout << "Caught exception in suggestions: " << e.what() << std::endl;
  }
}

int main()
{
  //Create server.
  Server srv;

  //Load corpus metadata from disc.
  //TODO (fux): location should be specified by a config file.
  std::ifstream read("bin/zotero.json", std::ios::in);

  //Check if metadata was found.
  if(!read) {
    std::cout << "No metadata found! Server fails to load\n";
    return 1;
  }
  nlohmann::json metadata;
  read >> metadata;

  //Load active pillars 
  //TODO (fux): these should be specified by a config file.
  std::cout << "TODO (fux): these should be specified by a config file.\n";

  nlohmann::json zotero_pillars = {
      {{"key","XCFFDRQC"}, {"name", "Forschung CLAS"}},
      {{"key", "RFWJC42V"}, {"name", "Geschichte des Tierwissens"}} }; 

  //Create book manager
  std::cout << "initializing bookmanager\n";
  CBookManager manager;
  manager.updateZotero(metadata);
  if (manager.initialize())
    std::cout << "Initialization successful!\n"; 
  else
    std::cout << "Initialization failed!\n";
  
  int start_port = std::stoi("4848");
  std::cout << "Starting on port: " << start_port << std::endl;

  // TODO (fux):  add signal-handler to stop the service.
  std::cout<<  "TODO (fux):  add signal-handler to stop the service.\n";
  
  //Add specific handlers via server-frame 
  srv.Get("/api/v2/search", [&](const Request& req, Response& resp) 
      { Search(req, resp, zotero_pillars, manager); });
  srv.Get("/api/v2/search/pages", [&](const Request& req, Response& resp) 
      { Pages(req, resp, manager); });
  srv.Get("/api/v2/search/suggestions/corpus", [&](const Request& req, Response& resp)
      { Suggestions(req, resp, manager, "corpus"); });
  srv.Get("/api/v2/search/suggestions/author", [&](const Request& req, Response& resp)
      { Suggestions(req, resp, manager, "author"); });

  //TODO (fux): this should be handled from main server
  srv.Get("/api/v2/search/pillars", [&](const Request& req, Response& resp)
      { resp.set_content(zotero_pillars.dump(), "application/json"); });

  //Serve static pages ONLY FOR TESTING!
  srv.Get("/", [](const Request& req, Response& resp) 
      { resp.set_content(GetPage("Search.html"), "text/html"); });
  srv.Get("/search(.*)", [](const Request& req, Response& resp)
      { resp.set_content(GetPage("Search.html"), "text/html"); });
  srv.Get("/Search.js", [](const Request& req, Response& resp) 
      { resp.set_content(GetPage("Search.js"), "application/javascript"); });
  srv.Get("/topnav.css", [](const Request& req, Response& resp) 
      { resp.set_content(GetPage("topnav.css"), "text/css"); });
  srv.Get("/Search.css", [](const Request& req, Response& resp) 
      { resp.set_content(GetPage("Search.css"), "text/css"); });
  std::cout << "C++ Api server startup successfull!" << std::endl;

  srv.listen("0.0.0.0", start_port);

  return 0;
}
