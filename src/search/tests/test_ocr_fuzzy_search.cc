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

BaseData* BaseData::instance_ = 0;


SCENARIO ("Searching for hard words with fuzzysearch", "[ocr_fuzzy]") {

  GIVEN ("An existing index based on a actual ocr and fuzzy-search") {

    BaseData* base_data = BaseData::instance("test_books", "metadata");

    // Initialize basic search_options.
    SearchOptions search_options(true, false, true, 0, 2070,
        0, "", {"XCFFDRQC"});

    WHEN ("Searching for 'hund'") {
      util::CheckResultsForQuery("hund", search_options, base_data);
    }

    WHEN ("Searching for 'Splitz+Pommer'") {
      util::CheckResultsForQuery("Splitz+Pommer", search_options, base_data);
    }

    WHEN ("Searching for 'Paradox'") {
      util::CheckResultsForQuery("Paradox", search_options, base_data);
    }

    WHEN ("Searching for 'domesticon'") {
      util::CheckResultsForQuery("domesticon", search_options, base_data);
    }

    WHEN ("Searching for 'schwedische Naturforscher'") {
      util::CheckResultsForQuery("schwedische Naturforscher", search_options, base_data);
    }

    WHEN ("Searching for 'äußere'") {
      util::CheckResultsForQuery("äußere", search_options, base_data);
    }

    WHEN ("Searching for 'Erdball'") {
      util::CheckResultsForQuery("Erdball", search_options, base_data);
    }
  }
}

SCENARIO("Searching for metadata", "[metadata]" ) {

  GIVEN ("An existing index based on a metadata normal-search") {

    BaseData* base_data = BaseData::instance("test_books", "metadata");

    // Initialize basic search_options.
    SearchOptions search_options(false, true, false, 0, 2070,
        0, "", {"XCFFDRQC"});

    WHEN ("Searching for 'Genealogie'") {
      util::CheckResultsForQuery("Genealogie", search_options, base_data);
    }

    WHEN ("Searching for 'der'") {
      util::CheckResultsForQuery("der", search_options, base_data);
    }

    WHEN ("Searching for 'Moral'") {
      util::CheckResultsForQuery("Moral", search_options, base_data);
    }

    WHEN ("Searching for '1999'") {
      util::CheckResultsForQuery("Moral", search_options, base_data);
    }

    WHEN ("Searching for 'Nietzsche'") {
      util::CheckResultsForQuery("Moral", search_options, base_data);
    }

    WHEN ("Searching for 'Nietzsche+1999'") {
      util::CheckResultsForQuery("Moral", search_options, base_data);
    }

    WHEN ("Searching for 'Moral+1999'") {
      util::CheckResultsForQuery("Moral", search_options, base_data);
    }

    WHEN ("Searching for 'Moral+Nietzsche'") {
      util::CheckResultsForQuery("Moral", search_options, base_data);
    }

    WHEN ("Searching for 'Moral+Nietzsche+1999'") {
      util::CheckResultsForQuery("Moral", search_options, base_data);
    }
  }
}
