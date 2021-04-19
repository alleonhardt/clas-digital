#ifndef SRC_SEARCH_TESTS_BASEDATA_H_
#define SRC_SEARCH_TESTS_BASEDATA_H_

#include "book_manager/book_manager.h"
#include "func.hpp"
#include "gramma.h"
#include "nlohmann/json.hpp"
#include <cstddef>
#include <iostream>
#include <string>

class BaseData {
  public:

    // getter:
    static BaseData *ocr_instance(std::string path_to_example_data) {
      if (!ocr_instance_)
        ocr_instance_ = new BaseData(path_to_example_data);
      return ocr_instance_; 
    }

    static BaseData *wiki_instance(std::string path_to_example_data) {
      if (!wiki_instance_)
        wiki_instance_ = new BaseData(path_to_example_data);
      return wiki_instance_; 
    }

    Dict& dict() { return dict_; }

    BookManager& book_manager() { return book_manager_; }

    std::string book_key() { return book_key_; }

    // setter 
    void set_book_key(std::string book_key) { book_key_ = book_key; }


    void SafeStatistic() {
      nlohmann::json all_statistics = func::LoadJsonFromDisc(base_path_ + path_to_example_data_ + "/stats.json");
      all_statistics[run_id_] = statics_;
      func::WriteContentToDisc(base_path_ + path_to_example_data_ + "/stats.json", all_statistics.dump());
    }

    void AddSearchStatistic(std::string query, size_t num_results, double seconds, double seconds_with_preview) {
      nlohmann::json new_search_stats;
      new_search_stats["query"] = query;
      new_search_stats["num_results"] = num_results;
      new_search_stats["elapsed_seconds"] = seconds;
      new_search_stats["elapsed_seconds_with_preview"] = seconds_with_preview;
      statics_["search"].push_back(new_search_stats);
      SafeStatistic();
    }

  private:
    static BaseData* ocr_instance_;
    static BaseData* wiki_instance_;
    Dict dict_;
    const std::string base_path_;
    BookManager book_manager_;
    std::string book_key_;
    const std::string path_to_example_data_;
    std::string run_id_;
    nlohmann::json statics_;

    BaseData(std::string path_to_example_data)
      : dict_("web/dict.json"), 
      base_path_("src/search/tests/example_data/"),
      book_manager_(
          {base_path_ + path_to_example_data + "/test_books"}, 
          &dict_, 
          func::LoadJsonFromDisc(base_path_ + path_to_example_data + "/parse_config.json"), 
          "search_data/" + path_to_example_data + ".db"), 
      book_key_(""),
      path_to_example_data_(path_to_example_data) {
    
      // Give book access to dictionary.
      Book::set_dict(&dict_);

      // Initialize book-manager.
      std::cout << "Loading metadata." << std::endl;
      nlohmann::json j_metadata = 
        func::LoadJsonFromDisc(base_path_ + path_to_example_data + "/metadata.json");
      std::cout << "Done." << std::endl;
      book_manager_.CreateItemsFromMetadata(j_metadata["items"]["data"], true);
      std::cout << "Initializing books." << std::endl;
      book_manager_.Initialize(true);
      
      // Get current time in human redable format
      std::cout << "Initializing new static entry." << std::endl;
      std::string cur_time = func::GetCurrentTime();
      statics_["time"] = cur_time;
      statics_["words"] = book_manager_.index_map().size();
      statics_["search"] = nlohmann::json::array();
      std::cout << "Done" << std::endl;

      run_id_ = cur_time.substr(0, cur_time.find(":"));
    }
};

#endif
