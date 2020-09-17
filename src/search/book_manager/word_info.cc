#include "word_info.h"

/**
 * Default Constructor.
 */
WordInfo::WordInfo() {
  position_ = 0;
  relevance_ = 0;
}
/**
 * Constructor.
 * @param[in] pages (vector of all pages word occures on)
 * @param[in] pos (first position the word occures in book)
 * @param[in] rel (relevance of this word)
 */
WordInfo::WordInfo(std::vector<size_t> pages, size_t pos, int rel) {
  pages_ = pages;
  position_ = pos;
  relevance_ = rel;
}

std::vector<size_t> WordInfo::pages() {
  return pages_;
}
size_t WordInfo::position() {
  return position_;
}
int WordInfo::relevance() {
  return relevance_;
}

//Setter
void WordInfo::set_pages(std::vector<size_t> pages) {
  pages_ = pages;
}
void WordInfo::set_position(size_t pos) {
  position_ = pos;
}
void WordInfo::set_relevance(int rel) {
  relevance_ = rel;
}

//Functions
void WordInfo::AddPage(size_t page) {
  pages_.push_back(page);
};

void WordInfo::AddRelevance(int val) {
  relevance_ += val*(val+1)/2;
}
