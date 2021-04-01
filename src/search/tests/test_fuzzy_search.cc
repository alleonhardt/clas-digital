#include <catch2/catch.hpp>
#include <cstddef>
#include <sys/types.h>
#include <utility>
#include "fuzzy.hpp"


TEST_CASE ("Test fuzzy-search", "[test-fuzzysearch]") {

  std::vector<std::pair<std::string, std::string>> words = {
    //{"hunde", "hande"},
    {"hund", "hande"},
    //{"jahrhundert", "hund"},
    //{"philosophies", "philosophie"},
    //{"philosophies", "philopophie"},
    //{"philopophie", "dphilosophies"}
  };

  for (const auto& it : words) {
    double score = fuzzy::fuzzy_cmp(it.first, it.second);
    std::cout << it.first << "~" << it.second << "=" << score << std::endl;
    REQUIRE(score <= 0.2);
  }
}

TEST_CASE ("Test new fuzzy-search", "[test-fuzzysearch]") {

  std::vector<std::vector<std::string>> words = {
    {"hunde", "hande", "1"},
    {"hund", "jahrhundert", "2"},
    {"philosophie", "philosophies", "1"},
    {"philopophie", "philosophies", "2"},
    {"philopophie", "dphilosophies", "3"}
  };

  for (const auto& it : words) {
    double score = fuzzy::fuzzy_cmp2(it[0], it[1]);
    std::cout << it[0] << "~" << it[1] << "=" << score << std::endl;
    REQUIRE(score == std::stoi(it[2]));
  }
}

