#include "util.h"
#include <iostream>


namespace util {
  void CheckResultsForQuery(std::string query, SearchOptions search_options, BaseData* base_data) {

    // Create search-object from given parameters and do search.
    SearchObject search_object = SearchObject(query, search_options, base_data->dict());
    auto results = base_data->book_manager().DoSearch(search_object);

    std::cout << "When search for " << query << " initialy found " << results.size() << " results." << std::endl;

    // Filter all results, where book_id is wrong and which are not found in corpus.
    auto it = std::find_if(results.begin(), results.end(), [&](ResultObject res_obj) { 
        // If search only in corpus, book needs to be found in corpus.
        if (search_options.only_corpus() && !res_obj.found_in_corpus_)
          return false;
        // If search only in metadata, book needs to be found in metadata.
        else if (search_options.only_metadata() && !res_obj.found_in_metadata())
         return false;

        // Book key need to match given book key.
        return res_obj.book()->key() == base_data->book_key(); 
      });

    // Check that search-results are correct.
    THEN (query + " is found and a preview is created.") {
      REQUIRE(it != results.end());

      if (search_options.only_metadata() == false) {
        std::string preview = results.front().book()->GetPreview(search_object, true);
        std::cout << "Preview for: " << base_data->book_key() << preview << std::endl;
        REQUIRE(preview.find("mark") != std::string::npos);
      }
    }
  }

  void CheckResultsForQuery2(std::string query, SearchOptions search_options, BaseData* base_data) {

    // Create search-object from given parameters and do search.
    auto start = std::chrono::system_clock::now();
    SearchObject search_object = SearchObject(query, search_options, base_data->dict());
    auto results = base_data->book_manager().DoSearch(search_object);

    std::cout << "When search for " << query << " initialy found " << results.size() << " results." << std::endl;

    // Check that search-results are correct.
    THEN (query + " is found and a preview is created.") {
      REQUIRE(results.size() > 0);

      for (auto it : results) {
        if (it.found_in_corpus()) {
          std::string preview = results.front().book()->GetPreview(search_object, true);
          std::cout << "Preview for: " << base_data->book_key() << preview << std::endl;
          REQUIRE(preview.find("mark") != std::string::npos);
        }
      }
    }
     auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s\n";
  }
}
