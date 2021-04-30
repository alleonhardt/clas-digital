#include "util.h"
#include "catch2/catch.hpp"
#include "func.hpp"
#include "result_object.h"
#include <iostream>


namespace util {
  std::list<ResultObject> CheckResultsForQuery(std::string query, 
      SearchOptions search_options, BaseData* base_data) {

    // Create search-object from given parameters and do search.
    auto start = std::chrono::system_clock::now();
    SearchObject search_object = SearchObject(query, search_options, base_data->dict());
    auto results = base_data->book_manager().Search(search_object, 10000);
    
    std::cout << "When search for " << query << " initialy found " << results.size() << " results." << std::endl;

    // Filter all results, where book_id is wrong and which are not found in corpus.
    auto it = std::find_if(results.begin(), results.end(), [&](ResultObject res_obj) { 
        // If a book-key is set, then book key needs to match given book key.
        if (base_data->book_key() != "")
          return res_obj.book()->key() == base_data->book_key(); 

        // If searched in corpus and metadata, we can return true
        if (search_options.only_corpus() && search_options.only_metadata())
          return true;

        // If searching only in corpus, book needs to be found in corpus.
        if (search_options.only_corpus() && !res_obj.found_in_corpus()) {
          return false;
        }

        // If searching only in metadata, book needs to be found in metadata.
        else if (search_options.only_metadata() && !res_obj.found_in_metadata()) {
          return false;
        }

        return true;
      });

    REQUIRE(it != results.end());
    // Get preview for each result and assert, that a preview was found in each result.
    for (auto it : results) {
      std::string preview = it.book()->GetPreview(
        it.matches_as_list(),
        search_options.fuzzy_search() 
      );
      REQUIRE(preview.find("mark") != std::string::npos);
      //it.Print("query", preview);
    }

    return results;
  }

  bool CheckAuthors(std::list<ResultObject>& results, std::string author) {
    for (const auto& it : results) {
        if (author != "") {
          if (it.book()->authors().count(author) == 0)
            return false;
        }
    }
    return true;
  }

  bool CheckSorting(std::list<ResultObject>& results, int sort_style) {
    double prev_score = 100000;
    int prev_date = 0;
    std::string prev_name = "aaaaaaaaaaa";
    for (const auto& it : results) {
      if (sort_style == 0) {
        if (it.score() > prev_score) {
          return false;
        }
        prev_name = it.score();
      }
      else if (sort_style == 1) {
        int date = it.book()->date();
        if (date == -1) date = 2050;
        if (date < prev_date) {
          return false;
        }
        prev_date = it.book()->date();
      }
      else if (sort_style == 2) {
        std::string author = it.book()->GetFromMetadata("authors");
        func::ConvertToLower(author);
        if (author < prev_name) {
          std::cout << author << " < " << prev_name << std::endl;
          return false;
        }
        prev_name = it.book()->GetFromMetadata("authors");
      }
    }
    return true;
  }
}
