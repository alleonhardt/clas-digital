#include <catch2/catch.hpp>
#include <cstddef>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>
#include "func.hpp"
#include "fuzzy.hpp"
#include "nlohmann/json.hpp"


TEST_CASE ("Test new fuzzy-search", "[test-fuzzysearch]") {

  std::vector<std::vector<std::string>> words = {
    {"hunde", "hunde", "0"},
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
    {"philopophie", "dphilosophies", "-1"},
    {"tone", "top", "-1"}
  };

  for (const auto& it : words) {
    double score = fuzzy::cmp(it[0], it[1]);
    std::cout << it[0] << "~" << it[1] << "=" << score << std::endl;
    REQUIRE(score == std::stoi(it[2]));
  }
}

std::map<std::string, std::pair<int, double>> cal_res(std::vector<std::string>& querys, std::vector<std::string>& words) {
  // Map of results.
  std::map<std::string, std::pair<int, double>> results;

  for (size_t i=0; i<querys.size(); i++) {
    std::cout << "Searching for " << querys[i] << std::endl;
    int found = 0;
    auto start = std::chrono::system_clock::now();
    for (size_t j=0; j<words.size(); j++) {
      int score = 0;
      short len_input = querys[i].length();
      short len_given = words[j].length();

      int threshold = (len_input > 8) ? 2 : 1;
      if (std::abs(len_input-len_given) > threshold)
        continue;
      if (len_given > len_input) 
        score = fuzzy::lshtein(querys[i].c_str(), words[j].c_str(), len_given, len_input, threshold);
      else 
        score = fuzzy::lshtein(querys[i].c_str(), words[j].c_str(), len_input, len_given, threshold);
      if (score > -1) {
        found++;
      }
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << querys[i] << " found " << found << " times in " << elapsed_seconds.count() << " seconds\n";
    results[querys[i]] = {found, elapsed_seconds.count()};
  }
  return results;
}

std::map<std::string, std::pair<int, double>> fast(std::vector<std::string>& querys, std::vector<std::string>& words) {
  // Map of results.
  std::map<std::string, std::pair<int, double>> results;

  for (size_t i=0; i<querys.size(); i++) {
    std::cout << "Searching for " << querys[i] << std::endl;
    int found = 0;
    auto start = std::chrono::system_clock::now();
    for (size_t j=0; j<words.size(); j++) {
      int score = 0;
      short len_input = querys[i].length();
      short len_given = words[j].length();

      int threshold = (len_input > 8) ? 2 : 1;
      if (len_input > 4 && threshold == 2) {
        std::string short_in = querys[i].substr(0,5);
        std::string short_gi = words[j];
        if (len_given > 4)
          short_gi = short_gi.substr(0,5);
        if (std::abs(len_input-len_given) > threshold)
          continue;
        if (len_given > len_input) 
          score = fuzzy::lshtein(short_in.c_str(), short_gi.c_str(), short_in.length(), short_gi.length(), 1);
        else 
          score = fuzzy::lshtein(short_gi.c_str(), short_in.c_str(), short_gi.length(), short_in.length(), 1);
        if (score == -1)
          continue;
      }

      if (std::abs(len_input-len_given) > threshold)
        continue;
      if (len_given > len_input) 
        score = fuzzy::lshtein(querys[i].c_str(), words[j].c_str(), len_given, len_input, threshold);
      else 
        score = fuzzy::lshtein(words[j].c_str(), querys[i].c_str(), len_input, len_given, threshold);
      if (score > -1) {
        found++;
      }
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << querys[i] << " found " << found << " times in " << elapsed_seconds.count() << " seconds\n";
    results[querys[i]] = {found, elapsed_seconds.count()};
  }
  return results;
}

TEST_CASE("Fuzzy-search-variations", "[test-fuzzysearch-variations]") {

  // Get all words. 
  nlohmann::json all_words_json = func::LoadJsonFromDisc("all_words_referance.json");
  std::vector<std::string> words;
  for (size_t i=0; i<10; i++) {
    for (const auto& it : all_words_json) {
      words.push_back(it[0]);
    }
  }

  std::cout << "words: " << words.size() << std::endl;
  

  // List of words to search for
  std::vector<std::string> querys = {"hund", "philopophie", "nachtigall", "hundert", "schlange",
    "springbrunnen", "tier", "Mensch", "Löwe", "Löwenzahn"};

  std::cout << "Search-terms: " << querys.size() << std::endl;

  auto results = cal_res(querys, words);
  auto results2 = fast(querys, words);

  std::cout << std::endl;
  for (const auto& it : results) {
    std::cout << it.first << ": " << it.second.first << ", " << it.second.second << " | ";
    auto sec = results2[it.first];
    double speed = it.second.second / sec.second;
    std::cout << it.first << ": " << sec.first << ", " << sec.second << " | ";
    std::cout << speed << " times as fast." << std::endl;
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

