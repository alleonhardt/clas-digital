/**
 * @author: fux.
 */

#ifndef SRC_SEARCH_BOOKMANAGER_WORDINFO_H_
#define SRC_SEARCH_BOOKMANAGER_WORDINFO_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

class WordInfo {
  public:
    WordInfo();

    std::vector<size_t> pages();
    int position();
    size_t relevance();

  private: 
    std::vector<size_t> pages_;
    std::map<std::stirng, std::vector<size_t>> neighbors_;
    int position_;
    size_t relevance_;
}

