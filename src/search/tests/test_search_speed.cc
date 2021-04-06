#include <chrono>
#include <list.h>
#include <iostream>
#include <string>

#include "base_data.h"

BaseData* BaseData::wiki_instance_ = 0;

void CheckResultsForQuery(std::string query, SearchOptions search_options, BaseData* base_data) {

    // Create search-object from given parameters and do search.
    auto start = std::chrono::system_clock::now();
    SearchObject search_object = SearchObject(query, search_options, base_data->dict());
    auto results = base_data->book_manager().Search(search_object);
    
    // Caluculate elapsed seconds.
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    double seconds_without_preview = elapsed_seconds.count();
    
    // Get preview for each result and assert, that a preview was found in each result.
    for (auto it : results) {
      std::string preview = it.book()->GetPreview(
        it.matches_as_list(),
        search_options.fuzzy_search() 
      );
    }

    // Caluculate elapsed seconds with preview
    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;
    std::cout << "Completed generating previews after: " << elapsed_seconds.count() << std::endl;
    std::cout << "Found " << results.size() << " results." << std::endl;
    base_data->AddSearchStatistic(query, results.size(), seconds_without_preview, 
        elapsed_seconds.count());
}

int main() {
  BaseData* base_data = BaseData::wiki_instance("wiki_data");
  
  // Initialize basic search_options.
  SearchOptions search_options(true, true, true, 0, 2070, 0, "", {"XCFFDRQC"});

    CheckResultsForQuery("Longacre", search_options, base_data);

    CheckResultsForQuery("Mahadewi", search_options, base_data);

    CheckResultsForQuery("André", search_options, base_data);

    CheckResultsForQuery("inklination", search_options, base_data);

    CheckResultsForQuery("sexual", search_options, base_data);

    CheckResultsForQuery("morality", search_options, base_data);

    CheckResultsForQuery("england", search_options, base_data);

    CheckResultsForQuery("Islamic+morality", search_options, base_data);

    CheckResultsForQuery("education+england", search_options, base_data);

    CheckResultsForQuery("education+england+islamic", search_options, base_data);

    CheckResultsForQuery("and", search_options, base_data);

    CheckResultsForQuery("xxx", search_options, base_data);
}
