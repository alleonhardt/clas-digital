#include "util.h"
#include "catch2/catch.hpp"
#include <iostream>


namespace util {
  void CheckResultsForQuery(std::string query, SearchOptions search_options, BaseData* base_data) {

    // Create search-object from given parameters and do search.
    auto start = std::chrono::system_clock::now();
    SearchObject search_object = SearchObject(query, search_options, base_data->dict());
    auto results = base_data->book_manager().Search(search_object);
    
    // Caluculate elapsed seconds.
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    double seconds_without_preview = elapsed_seconds.count();
    
    // std::cout << "When search for " << query << " initialy found " << results.size() << " results." << std::endl;

    // Filter all results, where book_id is wrong and which are not found in corpus.
    auto it = std::find_if(results.begin(), results.end(), [&](ResultObject res_obj) { 
        // If a book-key is set, then book key need to match given book key.
        if (base_data->book_key() != "")
          return res_obj.book()->key() == base_data->book_key(); 

        // if searched in corpus and metadata, we can return true
        if (search_options.only_corpus() && search_options.only_metadata())
          return true;

        // If search only in corpus, book needs to be found in corpus.
        if (search_options.only_corpus() && !res_obj.found_in_corpus())
          return false;
        // If search only in metadata, book needs to be found in metadata.
        else if (search_options.only_metadata() && !res_obj.found_in_metadata())
          return false;

        return true;
      });

    // Check that search-results are correct.
    THEN (query + " is found and a preview is created.") {
      // Require that results where found and that after filtering, results still exist.
      REQUIRE(it != results.end());
    }

    THEN ("Preview was created for each result.") {
      // Get preview for each result and assert, that a preview was found in each result.
      for (auto it : results) {
        // std::cout << "Creating preview for " << it.book()->key() << std::endl;
        std::string preview = it.book()->GetPreview(
          it.GetSearchWords(search_object.converted_to_original()),
          search_options.fuzzy_search() 
        );
        REQUIRE(preview.find("mark") != std::string::npos);
        std::cout << "Preview: " << preview << std::endl;
      }
    }

    // Caluculate elapsed seconds.
    auto end2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds2 = end2-start;
    base_data->AddSearchStatistic(query, results.size(), seconds_without_preview, elapsed_seconds2.count());
  }
}
