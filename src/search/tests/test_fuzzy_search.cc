#include <catch2/catch.hpp>
#include <cstddef>
#include <sys/types.h>
#include <utility>
#include "fuzzy.hpp"


TEST_CASE ("Test new fuzzy-search", "[test-fuzzysearch]") {

  std::vector<std::vector<std::string>> words = {
    {"hunde", "hande", "1"},
    {"hund", "jahrhundert", "2"},
    {"hund", "jagdhund", "1"},
    {"hund", "hundsfanger", "1"},
    {"hund", "und", "1"},
    {"hund", "jagdhund", "1"},
    {"jagd", "jagdhund", "1"},
    {"hund", "jahrhundert", "2"},
    {"philosophie", "philosophies", "1"},
    {"philopophie", "philosophies", "2"},
    {"philopophie", "dphilosophies", "-1"}
  };

  for (const auto& it : words) {
    double score = fuzzy::cmp(it[0], it[1]);
    std::cout << it[0] << "~" << it[1] << "=" << score << std::endl;
    REQUIRE(score == std::stoi(it[2]));
  }
}

