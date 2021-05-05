#ifndef SRC_SEARCH_STATISTICS_STATISTICS_H_
#define SRC_SEARCH_STATISTICS_STATISTICS_H_

#include <iostream>
#include <map>
#include <string>

#include "func.hpp"

struct SearchEntry {
  int fuzzy_searchs_;
  int normal_searchs_;
  int all_results_;
  double avarege_search_time_;
  double avarege_search_time_fuzzy_;
};

struct ManagerEntry {
  int num_documents_;
  int num_words_;
};

struct SearchStatistic {
  std::map<std::string, std::map<std::string, SearchEntry>> search_stats_;
  std::map<std::string, ManagerEntry> manager_stats_;
  size_t counter_ = 0;

  void SetManagerStats(int documents, int words) {
    manager_stats_[func::GetCurrentDate()] = {documents, words};
  }

  void AddEntry(std::string query, bool fuzzy, bool all_results, double time) {
    std::string cur_date = func::GetCurrentDate();
    if (search_stats_.count(cur_date) == 0)
      search_stats_[cur_date][query] 
        = {fuzzy==true, fuzzy==false, (all_results==true)*fuzzy, time*(fuzzy==false), time*(fuzzy==true)};
    else if (search_stats_[cur_date].count(query) == 0)
      search_stats_[cur_date][query] 
        = {fuzzy==true, fuzzy==false, (all_results==true)*fuzzy, time*(fuzzy==false), time*(fuzzy==true)};
    else {
      SearchEntry entry = search_stats_[cur_date][query];
      entry.fuzzy_searchs_ += fuzzy==true;
      entry.normal_searchs_ += (fuzzy==false);
      entry.all_results_ += all_results==true*fuzzy; // increase if fuzzy-search and all-results used.
      if (fuzzy)
        entry.avarege_search_time_fuzzy_ = (entry.avarege_search_time_fuzzy_+time)/2.0;
      else
        entry.avarege_search_time_ = (entry.avarege_search_time_+time)/2.0;
      search_stats_[cur_date][query] = entry;
    }

    if (++counter_ > 100) {
      SafeStats();
      search_stats_.clear();
      counter_ = 0;
    }
  }

  nlohmann::json GetAllStats() {
    std::cout << "GetAllStats" << std::endl;
    nlohmann::json stats = func::LoadJsonFromDisc("search_data/stats.json");

    // Update search-stats.
    for (const auto& it : search_stats_) {
      if (!stats.contains(it.first))
        stats[it.first] = nlohmann::json::object();
      for (const auto& jt : it.second) {
        nlohmann::json entry = nlohmann::json::object();

        // If an entry already exists, get existing entry.
        if (stats.count(it.first) > 0 && stats[it.first].contains(jt.first))
          entry = stats[it.first][jt.first];

        // Add new values.
        entry["fuzzy_searchs"] = entry.value("fuzzy_searchs", 0) + jt.second.fuzzy_searchs_;
        entry["normal_searchs"] = entry.value("normal_searchs", 0) + jt.second.normal_searchs_;
        entry["all_results"] = entry.value("all_results", 0) + jt.second.all_results_;
        entry["avarege_search_time"] = 
          (entry.value("avarege_search_time", jt.second.avarege_search_time_) + jt.second.avarege_search_time_)/2;
        entry["avarege_search_time_fuzzy"] 
          = (entry.value("avarege_search_time_fuzzy", jt.second.avarege_search_time_fuzzy_)
          + jt.second.avarege_search_time_fuzzy_)/2;
        stats[it.first][jt.first] = entry;
      }
    }

    // Updated manager stats.
    for (const auto& it : manager_stats_) {
      if (stats.contains(it.first))
        stats[it.first]["manager"] = {{"num_documents", it.second.num_documents_}, {"num_words", it.second.num_words_}};
      else {
        stats[it.first] = nlohmann::json::object();
        stats[it.first]["manager"] = {{"num_documents", it.second.num_documents_}, {"num_words", it.second.num_words_}};
      }
    }

    return stats;
  }

  void SafeStats() {
    std::cout << "SafeStats()" << std::endl;
    nlohmann::json stats = GetAllStats();
    func::WriteContentToDisc("search_data/stats.json", stats.dump());
  }
};

#endif
