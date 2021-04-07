#include <algorithm>
#include <bits/stdint-uintn.h>
#include <catch2/catch.hpp>
#include <fstream>
#include <vector>
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

BaseData* BaseData::ocr_instance_ = 0;


SCENARIO ("Searching for hard words with fuzzysearch", "[ocr_fuzzy]") {

  GIVEN ("An existing index based on a actual ocr and fuzzy-search") {

    BaseData* base_data = BaseData::ocr_instance("ocr_data");
    base_data->set_book_key("UEHEJINT");

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
    WHEN ("Searching for 'Genealogie'") {
      util::CheckResultsForQuery("Erdball", search_options, base_data);
    }
  }
}

SCENARIO("Searching for metadata", "[metadata]" ) {

  GIVEN ("An existing index based on a metadata normal-search") {

    BaseData* base_data = BaseData::ocr_instance("ocr_data");
    base_data->set_book_key("");

    // Initialize basic search_options.
    SearchOptions search_options(true, true, false, 0, 2070,
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
      util::CheckResultsForQuery("Nietzsche", search_options, base_data);
    }

    WHEN ("Searching for 'Nietzsche+1999'") {
      util::CheckResultsForQuery("Nietzsche+1999", search_options, base_data);
    }

    WHEN ("Searching for 'Moral+1999'") {
      util::CheckResultsForQuery("Moral+1999", search_options, base_data);
    }

    WHEN ("Searching for 'Moral+Nietzsche'") {
      util::CheckResultsForQuery("Moral+Nietzsche", search_options, base_data);
    }

    WHEN ("Searching for 'Moral+Nietzsche+1999'") {
      util::CheckResultsForQuery("Moral+Nietzsche+1999", search_options, base_data);
    }
    WHEN ("Searching for '1989'") {
      util::CheckResultsForQuery("1989", search_options, base_data);
    }
    WHEN ("Searching for 'Mistakes'") {
      util::CheckResultsForQuery("Mistakes", search_options, base_data);
    }
    WHEN ("Searching for 'Ingensiep'") {
      util::CheckResultsForQuery("Ingensiep", search_options, base_data);
    }
  }
}

SCENARIO("Test creating description for books", "[descriptions]") {

  GIVEN ("An existing index based on a actual ocr and fuzzy-search") {
    BaseData* base_data = BaseData::ocr_instance("ocr_data");
    base_data->set_book_key("UEHEJINT");

    for (const auto& it : base_data->book_manager().documents()) {

      // Get description.
      std::string description = it.second->GetFromMetadata("description");

      // Get author 
      std::vector<std::string> authors = func::split2(it.second->GetFromMetadata("authorsLastNames"), ";");
      std::string author_last_name = "undefined";
      if (authors.size() > 0)
        author_last_name = authors[0];
      if (author_last_name == "")
        author_last_name = "undefined";

      // Get date.
      std::string date = it.second->GetFromMetadata("date");
      if (date == "")
        date = "undefined";

      // Check that description was created as expected.
      std::cout << description << ": " << author_last_name << ", " << date << std::endl;
      REQUIRE(description == author_last_name + ", " + date);
    }
  }
}
