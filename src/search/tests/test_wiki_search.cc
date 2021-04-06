#include <algorithm>
#include <bits/stdint-uintn.h>
#include <catch2/catch.hpp>
#include <fstream>
#include "func.hpp"
#include "book_manager/book.h"
#include "base_data.h"
#include "book_manager/book_manager.h"
#include "nlohmann/json.hpp"
#include "gramma.h"
#include "result_object.h"
#include "search_object.h"
#include "search_options.h"
#include "util.h"

BaseData* BaseData::wiki_instance_ = 0;


SCENARIO ("Searching for wiki words with fuzzysearch", "[wiki_fuzzy]") {

  GIVEN ("An existing index based on a wikipedia entries and fuzzy-search") {

    BaseData* base_data = BaseData::wiki_instance("wiki_data");

    // Initialize basic search_options.
    SearchOptions search_options(true, true, true, 0, 2070, 0, "", {"XCFFDRQC"});

    WHEN ("Searching for 'Longacre'") {
      util::CheckResultsForQuery("Longacre", search_options, base_data);
    }

    WHEN ("Searching for 'Mahadevi'") {
      util::CheckResultsForQuery("Mahadewi", search_options, base_data);
    }

    WHEN ("Searching for 'André'") {
      util::CheckResultsForQuery("André", search_options, base_data);
    }

    WHEN ("Searching for 'inclination'") {
      util::CheckResultsForQuery("inklination", search_options, base_data);
    }

    WHEN ("Searching for 'sexual'") {
      util::CheckResultsForQuery("sexual", search_options, base_data);
    }

    WHEN ("Searching for 'morality'") {
      util::CheckResultsForQuery("morality", search_options, base_data);
    }

    WHEN ("Searching for 'england'") {
      util::CheckResultsForQuery("england", search_options, base_data);
    }

    WHEN ("Searching for 'Islamic morality'") {
      util::CheckResultsForQuery("Islamic+morality", search_options, base_data);
    }

    WHEN ("Searching for 'education england'") {
      util::CheckResultsForQuery("education+england", search_options, base_data);
    }
    WHEN ("Searching for 'and'") {
      util::CheckResultsForQuery("and", search_options, base_data);
    }
    WHEN ("Searching for 'xxx'") {
      util::CheckResultsForQuery("xxx", search_options, base_data);
    }
  }
}

SCENARIO ("Searching for wiki words - search options", "[wiki_fuzzy]") {

  GIVEN ("An existing index based on a wikipedia entries and fuzzy-search") {

    BaseData* base_data = BaseData::wiki_instance("wiki_data");

    // Initialize basic search_options.

    WHEN ("Searching for 'and' with specific author (first name)") {
      SearchOptions search_options(true, true, true, 0, 2070, 0, "Kandis", {"XCFFDRQC"});
      auto results = util::CheckResultsForQuery("and", search_options, base_data);
      REQUIRE(util::CheckAuthors(results, search_options.author()));
    }

    WHEN ("Searching for 'and' with specific author (last name)" ) {
      SearchOptions search_options(true, true, true, 0, 2070, 0, "Mclaughlin", {"XCFFDRQC"});
      auto results = util::CheckResultsForQuery("and", search_options, base_data);
      REQUIRE(util::CheckAuthors(results, search_options.author()));
    }

    WHEN ("Searching for 'and' with specific sorting: relevance") {
      SearchOptions search_options(true, true, true, 0, 2070, 0, "", {"XCFFDRQC"});
      auto results = util::CheckResultsForQuery("and", search_options, base_data);
      REQUIRE(util::CheckSorting(results, search_options.sort_results_by()));
    }

    WHEN ("Searching for 'and' with specific sorting: chronologically") {
      SearchOptions search_options(true, true, true, 0, 2070, 1, "", {"XCFFDRQC"});
      auto results = util::CheckResultsForQuery("and", search_options, base_data);
      REQUIRE(util::CheckSorting(results, search_options.sort_results_by()));
    }

    WHEN ("Searching for 'and' with specific sorting: alphabetically") {
      SearchOptions search_options(true, true, true, 0, 2070, 2, "", {"XCFFDRQC"});
      auto results = util::CheckResultsForQuery("and", search_options, base_data);
      REQUIRE(util::CheckSorting(results, search_options.sort_results_by()));
    }
  }
}

