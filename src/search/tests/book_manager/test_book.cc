#include <algorithm>
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>
#include <cstddef>
#include "book_manager/book.h"
#include "tmp_word_info.h"
#include <map>

#define private public

TempWordInfo create_word_info(std::vector<std::pair<size_t, size_t>> pages, 
    size_t preview_position, size_t preview_page) {

  TempWordInfo word_info;
  int raw_count = 0;
  for (auto weigthed_page : pages) {
    word_info.AddPage(weigthed_page);
    raw_count += weigthed_page.second;
  }
  word_info.set_preview_position(preview_position);
  word_info.set_preview_page(preview_page);
  word_info.IncreaseRawCount(raw_count);
  return word_info;
}

TEST_CASE( "Duplicates are joined as expected", "[handle_duplicates]" ) {

  std::map<std::string, TempWordInfo> tmp_index_map;
  tmp_index_map["Hund"] = create_word_info({{1,1,},{2,2},{4,3},{5,2}}, 10, 4);
  tmp_index_map["hund"] = create_word_info({{7,1},{8,2},{3,1},{5,2}}, 5, 8);
  Book book;
  book.ConvertWords(tmp_index_map);

  REQUIRE(tmp_index_map.count("Hund") == 0);
  REQUIRE(tmp_index_map.count("hund") > 0);
  REQUIRE(tmp_index_map["hund"].preview_position() == 10);
  REQUIRE(tmp_index_map["hund"].raw_count() == 14);
  std::vector<size_t> all_pages = tmp_index_map["hund"].GetAllPages();
  std::vector<size_t> excpected_pages = {1,2,3,4,5,7,8};
  REQUIRE(std::equal(all_pages.begin(), all_pages.end(), excpected_pages.begin()) == true);
}

TEST_CASE( "Words are converted to lower as expected", "[convert_words]" ) {
  std::map<std::string, TempWordInfo> tmp_index_map;
  tmp_index_map["Hündin"] = create_word_info({{1,1,},{2,2},{4,3},{5,2}}, 10, 4);
  tmp_index_map["Straße"] = create_word_info({{7,1},{8,2},{3,1},{5,2}}, 5, 8);
  Book book;
  book.ConvertWords(tmp_index_map);

  REQUIRE(tmp_index_map.count("Hündin") == 0);
  REQUIRE(tmp_index_map.count("hündin") > 0);
  REQUIRE(tmp_index_map.count("Straße") == 0);
  REQUIRE(tmp_index_map.count("straße") > 0);
}

TEST_CASE( "The map of base-forms is created as expected", "[generate_base_forms]") {
  Dict dict("web/dict.json");
  Book::set_dict(&dict);

  std::map<std::string, TempWordInfo> tmp_index_map;
  tmp_index_map["Hund"] = create_word_info({{1,1,},{2,2},{4,3},{5,2}}, 10, 4);
  tmp_index_map["hund"] = create_word_info({{7,1},{8,2},{3,1},{5,2}}, 5, 8);
  tmp_index_map["hunde"] = create_word_info({{7,1},{8,2},{3,1},{5,2}}, 5, 8);
  tmp_index_map["hundes"] = create_word_info({{7,1},{8,2},{3,1},{5,2}}, 5, 8);
  tmp_index_map["Hündin"] = create_word_info({{1,1,},{2,2},{4,3},{5,2}}, 10, 4);
  tmp_index_map["Strassen"] = create_word_info({{7,1},{8,2},{3,1},{5,2}}, 5, 8);
  Book book;
  book.GenerateBaseFormMap(tmp_index_map, book.words_on_pages_);

  REQUIRE(book.words_on_pages().count("hund") > 0);
  REQUIRE(book.words_on_pages()["hund"].size() == 3);

  REQUIRE(book.words_on_pages().count("strasse") > 0);
  REQUIRE(book.words_on_pages()["strasse"].size() == 1);
  REQUIRE(book.words_on_pages()["strasse"].front().word_ == "strassen");
}
