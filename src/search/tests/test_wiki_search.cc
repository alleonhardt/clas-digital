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

BaseData* BaseData::instance2_ = 0;


SCENARIO ("Searching for wiki words with fuzzysearch", "[wiki_fuzzy]") {

  GIVEN ("An existing index based on a wikipedia entries and fuzzy-search") {

    BaseData* base_data = BaseData::instance2("test_books_2", "metadata_2");

    // Initialize basic search_options.
    SearchOptions search_options(true, true, true, 0, 2070, 0, "", {"XCFFDRQC"});

    WHEN ("Searching for 'Longacre'") {
      util::CheckResultsForQuery2("Longacre", search_options, base_data);
    }

    WHEN ("Searching for 'Mahadevi'") {
      util::CheckResultsForQuery2("Mahadewi", search_options, base_data);
    }

    WHEN ("Searching for 'André'") {
      util::CheckResultsForQuery2("André", search_options, base_data);
    }

    WHEN ("Searching for 'inclination'") {
      util::CheckResultsForQuery2("inklination", search_options, base_data);
    }

    WHEN ("Searching for 'sexual'") {
      util::CheckResultsForQuery2("sexual", search_options, base_data);
    }
  }
}

