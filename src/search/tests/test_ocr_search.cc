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

    WHEN ("Searching for 'schwedische'") {
      util::CheckResultsForQuery("schwedische", search_options, base_data);
    }

    WHEN ("Searching for 'Naturforscher'") {
      util::CheckResultsForQuery("Naturforscher", search_options, base_data);
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
      std::vector<std::string> authors = func::Split2(it.second->GetFromMetadata("authorsLastNames"), ";");
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
      REQUIRE(description == author_last_name + ", " + date);
    }
  }
}

SCENARIO("Test finding pages and matches in one book", "[search in book]") {
  GIVEN ("An existing index based on a actual ocr and fuzzy-search") {

    BaseData* base_data = BaseData::ocr_instance("ocr_data");
    base_data->set_book_key("UEHEJINT");
    Book* book = base_data->book_manager().documents().at("UEHEJINT");
    
    // Initialize basic search_options.
    SearchOptions search_options(true);

    WHEN ("Searching for 'hund'") {
      SearchObject search_object = {"hund", search_options, base_data->dict()};
      auto pages = book->GetPages(search_object);
      
      // Check a selection of matches found in corpus.
      REQUIRE(pages.count(19) > 0);
      REQUIRE(std::find(pages[19].begin(), pages[19].end(), "seehunde") != pages[19].end());
      REQUIRE(pages.count(29) > 0);
      REQUIRE(std::find(pages[29].begin(), pages[29].end(), "hunde") != pages[29].end());
      REQUIRE(std::find(pages[29].begin(), pages[29].end(), "hunderttauſende") != pages[29].end());
      REQUIRE(pages.count(34) > 0);
      REQUIRE(std::find(pages[34].begin(), pages[34].end(), "seehunden") != pages[34].end());
      REQUIRE(pages.count(43) > 0);
      REQUIRE(std::find(pages[43].begin(), pages[43].end(), "seehunde") != pages[43].end());
      REQUIRE(pages.count(43) > 0);
      REQUIRE(std::find(pages[43].begin(), pages[43].end(), "seehunde") != pages[43].end());
    }

    WHEN ("Searching for 'volk'") {
      SearchObject search_object = {"volk", search_options, base_data->dict()};
      auto pages = book->GetPages(search_object);
      // Check a selection of matches found in corpus.
      REQUIRE(pages.count(60) > 0);
      REQUIRE(std::find(pages[60].begin(), pages[60].end(), "schiffsvolk") != pages[60].end());
      REQUIRE(pages.count(72) > 0);
      REQUIRE(std::find(pages[72].begin(), pages[72].end(), "bevölkerung") != pages[72].end());
    }

    WHEN ("Searching for 'volk+hund'") {
      SearchObject search_object = {"volk+hund", search_options, base_data->dict()};
      auto pages = book->GetPages(search_object);

      // All matches vor "volk" are still found.
      SearchObject search_object_volk = {"volk", search_options, base_data->dict()};
      auto pages_volk = book->GetPages(search_object_volk);
      for (const auto& it : pages_volk) {
        REQUIRE(pages.count(it.first) > 0);
        for (const auto& word : it.second)
          REQUIRE(std::find(pages[it.first].begin(), pages[it.first].end(), word) != pages[it.first].end());
      }

      // All matches vor "hund"  are still found.
      SearchObject search_object_hund = {"hund", search_options, base_data->dict()};
      auto pages_hund = book->GetPages(search_object_hund);
      for (const auto& it : pages_hund) {
        REQUIRE(pages.count(it.first) > 0);
        for (const auto& word : it.second)
          REQUIRE(std::find(pages[it.first].begin(), pages[it.first].end(), word) != pages[it.first].end());
      }

      // Check a selection of matches vor "hund" und "volk"
      REQUIRE(pages.count(109) > 0);
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "schiffsvolk") != pages[109].end());
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "seehunde") != pages[109].end());
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "seehunde") != pages[109].end());
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "hundert") != pages[109].end());
    }

    WHEN ("Searching for 'volk+hund+bär'") {
      SearchObject search_object = {"volk+hund+bär", search_options, base_data->dict()};
      auto pages = book->GetPages(search_object);

      // All matches vor "volk" are still found.
      SearchObject search_object_volk = {"volk", search_options, base_data->dict()};
      auto pages_volk = book->GetPages(search_object_volk);
      for (const auto& it : pages_volk) {
        REQUIRE(pages.count(it.first) > 0);
        for (const auto& word : it.second)
          REQUIRE(std::find(pages[it.first].begin(), pages[it.first].end(), word) != pages[it.first].end());
      }

      // All matches vor "hund"  are still found.
      SearchObject search_object_hund = {"hund", search_options, base_data->dict()};
      auto pages_hund = book->GetPages(search_object_hund);
      for (const auto& it : pages_hund) {
        REQUIRE(pages.count(it.first) > 0);
        for (const auto& word : it.second)
          REQUIRE(std::find(pages[it.first].begin(), pages[it.first].end(), word) != pages[it.first].end());
      }
      
      // All matches vor "bär"  are still found.
      SearchObject search_object_baer = {"bär", search_options, base_data->dict()};
      auto pages_baer = book->GetPages(search_object_hund);
      for (const auto& it : pages_baer) {
        REQUIRE(pages.count(it.first) > 0);
        for (const auto& word : it.second)
          REQUIRE(std::find(pages[it.first].begin(), pages[it.first].end(), word) != pages[it.first].end());
      }

      // Check a selection of matches vor "hund" und "volk"
      REQUIRE(pages.count(109) > 0);
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "schiffsvolk") != pages[109].end());
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "seehunde") != pages[109].end());
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "seehunde") != pages[109].end());
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "hundert") != pages[109].end());
      REQUIRE(std::find(pages[109].begin(), pages[109].end(), "eisbär") != pages[109].end());
    }
  }
}
