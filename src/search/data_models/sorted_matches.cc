#include "sorted_matches.h"
#include <vector>


SortedMatches::SortedMatches() : matches_(&cmp) {}

SortedMatches::SortedMatches(std::pair<std::string, double> first_match) 
  : matches_(&cmp) {
    matches_.insert(first_match); 
}

void SortedMatches::Insert(weighted_match new_match) {
  matches_.insert(matches_.begin(), new_match);
  if (matches_.size() > 10)
    matches_.erase(--matches_.end());
}

std::string SortedMatches::GetBestMatch() {
  return matches_.begin()->first;
}

std::vector<std::string> SortedMatches::GetAllMatches() {
  std::vector<std::string> all_matches;
  for (auto it : matches_) 
    all_matches.push_back(it.first);
  return all_matches;
}
