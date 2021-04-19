/**
 * @author fux
 * Class storing information of one word (indexing).
 */

#include <iostream>
#include <vector>

struct WordInfo {
  std::string word_;  ///< One occurance of a word (lower-case, non utf-8 characters removed).
  std::vector<size_t> pages_; ///< All pages the word occures on.
  size_t preview_position_; ///< Position of best preview.
  size_t preview_page_; ///< Page best preview is on.
  double term_frequency_;  ///< raw_count (occurences of word in document) / |D| (num (different) words in document)
};
