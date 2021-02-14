#include <algorithm>
#include <catch2/catch.hpp>
#include <fstream>
#include "func.hpp"
#include "book_manager/book.h"
#include "gramma.h"
#include "book_manager/book_manager.h"
#include "nlohmann/json.hpp"
#include "result_object.h"

bool verify_found(std::string query) {


}

TEST_CASE ("Search finds some words", "[finds_words]") {

  Dict dict("web/dict.json");
  Book::set_dict(&dict);

  BookManager manager({"src/search/tests/test_books"}, &dict);

  nlohmann::json metadata;
  std::ifstream read("src/search/tests/metadata.json");
  read >> metadata;
  read.close();
  manager.CreaeItemsFromMetadata(metadata);
  REQUIRE(manager.Initialize(true) == true);

  // Initialize basic search_options.
  SearchOptions search_options(true, true, true, 0, 2070,
      0, "", {"XCFFDRQC"});
  std::string book_id = "UEHEJINT";

  SECTION ("Finds 'hund'") {
    SearchObject search_object = SearchObject("hund", search_options, dict);
    auto results = manager.DoSearch(search_object);

    auto it = std::find_if(results.begin(), results.end(), 
        [&](ResultObject res_obj) { return res_obj.book_->key() == book_id; });
    REQUIRE(it != results.end());

    std::string preview = results.front().book_->GetPreview(search_object, true);
    std::cout << preview << std::endl;
    REQUIRE(preview.find("mark") != std::string::npos);
  }

  SECTION ("Finds 'Windhund+Fleischerhund'") {
    SearchObject search_object = SearchObject("Windhund+Fleischerhund", search_options, dict);
    auto results = manager.DoSearch(search_object);

    auto it = std::find_if(results.begin(), results.end(), 
        [&](ResultObject res_obj) { return res_obj.book_->key() == book_id; });
    REQUIRE(it != results.end());

    std::string preview = results.front().book_->GetPreview(search_object, true);
    std::cout << preview << std::endl;
    REQUIRE(preview.find("mark") != std::string::npos);
  }
  SECTION ("Finds 'Spitz+Pommer'") {
    SearchObject search_object = SearchObject("Spitz+Pommer", search_options, dict);
    auto results = manager.DoSearch(search_object);

    auto it = std::find_if(results.begin(), results.end(), 
        [&](ResultObject res_obj) { return res_obj.book_->key() == book_id; });
    REQUIRE(it != results.end());

    std::string preview = results.front().book_->GetPreview(search_object, true);
    std::cout << preview << std::endl;
    REQUIRE(preview.find("mark") != std::string::npos);
  }

  SECTION ("Finds 'Paradox'") {
    SearchObject search_object = SearchObject("paradox", search_options, dict);
    auto results = manager.DoSearch(search_object);

    auto it = std::find_if(results.begin(), results.end(), 
        [&](ResultObject res_obj) { return res_obj.book_->key() == book_id; });
    REQUIRE(it != results.end());

    std::string preview = results.front().book_->GetPreview(search_object, true);
    std::cout << preview << std::endl;
    REQUIRE(preview.find("mark") != std::string::npos);
  }
  SECTION ("Finds 'domesticon'") {
    SearchObject search_object = SearchObject("domesticon", search_options, dict);
    auto results = manager.DoSearch(search_object);

    auto it = std::find_if(results.begin(), results.end(), 
        [&](ResultObject res_obj) { return res_obj.book_->key() == book_id; });
    REQUIRE(it != results.end());

    std::string preview = results.front().book_->GetPreview(search_object, true);
    std::cout << preview << std::endl;
    REQUIRE(preview.find("mark") != std::string::npos);
  }
}
