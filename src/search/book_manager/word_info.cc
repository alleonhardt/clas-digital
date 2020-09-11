#include "word_info.h"

/**
 * Constructor.
 * @param[in] pages (vector of all pages word occures on)
 * @param[in] pos (first position the word occures in book)
 * @param[in] rel (relevance of this word)
 */
WordInfo::WordInfo(std::vector<size_t> pages, int pos, size_t rel) {
  pages_ = pages;
  position_ = pos;
  relevance_ = rel;
}

std::vector<size_t> WordInfo::pages() {
  return pages_;
}
int WordInfo::position() {
  return position_;
}
size_t WordInfo::relevance() {
  return relevance_;
}

//Setter
void WordInfo::set_pages(std::vector<size_t> pages) {
  pages_ = pages;
}
void WordInfo::set_position(int pos) {
  position_ = pos;
}
void WordInfo::set_relevance(size_t rel) {
  relevance_ = rel;
}
