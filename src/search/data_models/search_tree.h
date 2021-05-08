#ifndef SRC_SEARCH_DATA_MODELS_H_
#define SRC_SEARCH_DATA_MODELS_H_

#include <cstddef>
#include <map>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <queue>

#include "fuzzy.hpp"

class SearchTree {
  public:
    SearchTree(std::string word) {
      word_ = word;
      children_ = {};
    }
    
    std::string word() const {
      return word_;
    }

    std::vector<std::pair<size_t, SearchTree*>> children() const {
      return children_;
    }

    void Insert(std::string word) {
      size_t dist = fuzzy::LevenshteinDistance(word_.c_str(), word.c_str());
      for (const auto& it : children_) {
        if (it.first == dist) {
          it.second->Insert(word);
          return;
        }
      }
      children_.push_back({dist, new SearchTree(word)});
    }

    std::vector<std::string> SearchRecursive(std::string query, size_t threshold) {
      // Calculate distance.
      size_t dist = fuzzy::LevenshteinDistance(word_.c_str(), query.c_str());
      std::vector<std::string> results;
      if (dist <= threshold)
        results.push_back(word_);

      for (const auto& it : children_) {
        if (it.first-threshold <= dist && dist <= it.first+threshold) {
          auto new_results = it.second->SearchRecursive(query, threshold);
          results.insert(results.begin(), new_results.begin(), new_results.end());
        }
      }

      return results;
    }

    std::vector<std::string> SearchIterative(std::string query, size_t threshold) {
      short lenq = query.length();
      size_t counter=0;
      std::vector<std::string> results;
      std::queue<SearchTree*> nodes;
      nodes.push(this);
      while (nodes.size() > 0) {
        counter++;
        SearchTree* cur = nodes.front();
        nodes.pop();
        short lenw = cur->word().length();

        size_t dist = (lenq > lenw) ? lenq : lenw;
        if (std::abs(lenq - lenw) <= threshold)
          dist = fuzzy::lshteinNoStop(cur->word().c_str(), query.c_str(), lenq, lenw, threshold);
        if (dist <= threshold) 
          results.push_back(cur->word());

        for (const auto& it : cur->children()) {
          if (it.first-threshold <= dist && dist <= it.first+threshold)
            nodes.push(it.second);
        }
      }
      std::cout << counter << " elements checked." << std::endl;
      return results;
    }

  private:
    std::string word_;
    std::vector<std::pair<size_t, SearchTree*>> children_;
};

#endif
