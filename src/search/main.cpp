/**
 * @author fux
 */

#include <iostream>
#include <string>

#include <httplib.h>

#include "book_manager/CBookManager.hpp"

using namespace httplib;

/**
 * Search in all entries in corpus. And all metadata.
 *
 */
void Search(const Request& req, Response& resp, const nlohmann::json& 
    zotero_pillars, CBookManager& manager) {
  
  //Get query
  std::string query = req.get_param_value("q", 0);

  //TODO (fux): parse pillars!
  
  bool fuzzyness = false;
  if (req.has_param("fuzzyness")) 
    fuzzyness = std::stoi(req.get_param_value("fuzzyness", 0)) != 0;

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

  std::string scope = "all";
  if (req.has_param("scope")) 
    scope = req.get_param_value("scope", 0);

  std::string author = "";
  if (req.has_param("author"))
    author = req.get_param_value("author", 0);

  int pubafter = 1700, pubbefore = 2049;
  try{ pubafter = std::stoi(req.get_param_value("publicatedafter")); } 
  catch(...){};
  try{ pubbefore = std::stoi(req.get_param_value("publicatedbefore")); } 
  catch(...){};

  std::string sort = "relevance";
  if (req.has_param("sorting"))
    sort = req.get_param_value("sorting", 0);

  int resultsperpage = 10;
  try{ resultsperpage = std::stoi(req.get_param_value("limit", 0));}
  catch(...){};

  std::cout << "Recieved search request!" << std::endl;
  std::cout << "Query: " << query << 
    "; fuzzyness: " << fuzzyness <<
    "; scope: " << scope <<
    "; author: " << author <<
    "; publicated after: " << pubafter <<
    "; publicated before: " << pubbefore << 
    "; searched pillars: " << str_pillars << 
    "; vector pillar size: " << pillars.size() <<
    "; sorting with value: " << sort << " and max results per page: " 
    << resultsperpage << std::endl;

  CSearchOptions options(query, fuzzyness, pillars, scope, author, pubafter,
    pubbefore, true, sort);

  std::cout << "Started search!" << std::endl;

  auto result_list = manager.search(&options);
  nlohmann::json json_respond;

  if (result_list->size() == 0) {
    json_respond["max_results"] = 0;
    json_respond["time"] = 0;
  }

}

/**
 * Get pages.
 */
void Pages(const Request& req, Response& resp, CBookManager& manager) {

}

/**
 * Returns suggestions for authors
 */
void SuggestionsAuthor(const Request& req, Response& resp, CBookManager& manager) {

}
/**
 * Returns suggestions for words in corpus 
 */
void SuggestionsCorpus(const Request& req, Response& resp, CBookManager& manager) {

}

int main()
{
  //Create server.
  Server srv;

  //Load corpus metadata from disc.
  //TODO (fux): location should be specified by a config file.
  std::ifstream read("zotero,json", std::ios::in);

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
  srv.Get("/api/search/", [&](const Request& req, Response& resp) 
                  { Search(req, resp, zotero_pillars, manager); });
  srv.Get("/api/search/pages", [&](const Request& req, Response& resp) 
                  { Pages(req, resp, manager); });
  srv.Get("/api/suggestions/corpus", [&](const Request& req, Response& resp)
                  { SuggestionsCorpus(req, resp, manager); });
  srv.Get("/api/suggestions/author", [&](const Request& req, Response& resp)
                  { SuggestionsAuthor(req, resp, manager); });
  std::cout << "C++ Api server startup successfull!" << std::endl;

  srv.listen("0.0.0.0", start_port);

  return 0;
}
