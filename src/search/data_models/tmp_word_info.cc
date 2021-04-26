#include "tmp_word_info.h"
#include <cstddef>
#include <vector>

TempWordInfo::TempWordInfo() : pages_with_relevance_(&cmp), raw_count_(0), 
  preview_page_(0), preview_position_(0) {}

std::set<TempWordInfo::weighted_match, TempWordInfo::Cmp>& TempWordInfo::pages_with_relevance() {
  return pages_with_relevance_;
}
size_t TempWordInfo::preview_position() const {
  return preview_position_;
}
size_t TempWordInfo::preview_page() const {
  return preview_page_;
}
int TempWordInfo::raw_count() const {
  return raw_count_;
}

void TempWordInfo::set_preview_position(size_t pos) {
  preview_position_ = pos;
}

void TempWordInfo::set_preview_page(size_t page) {
  preview_page_ = page;
}

void TempWordInfo::AddPage(weighted_match page_with_relevance) {
  pages_with_relevance_.insert(pages_with_relevance_.begin(), page_with_relevance);
}

void TempWordInfo::IncreaseRawCount(int val) {
  raw_count_ += val;
}

size_t TempWordInfo::GetBestPage() {
  return pages_with_relevance_.begin()->first;
}
std::vector<size_t> TempWordInfo::GetAllPages() const {
  std::map<size_t, size_t> sorted_pages;
  for (auto it : pages_with_relevance_) 
    sorted_pages[it.first] = 0;
  std::vector<size_t> all_pages;
  for (auto it : sorted_pages) 
    all_pages.push_back(it.first);
  return all_pages;
}

void TempWordInfo::Join(TempWordInfo& word_info) {
  // Add up relevance of both word-infos.
  raw_count_ += word_info.raw_count();

  // Find preview_position with better relevance for page.
  size_t page_with_relevance_a = 0;
  for (auto it : pages_with_relevance_) {
    if (it.first == preview_page_) 
      page_with_relevance_a = it.second;
  }
  size_t page_with_relevance_b = 0;
  for (auto it : word_info.pages_with_relevance()) {
    if (it.first == word_info.preview_page()) 
      page_with_relevance_b = it.second;
  }
  // Set preview-position and preview-page accordingly.
  if (page_with_relevance_a < page_with_relevance_b) {
    preview_page_ = word_info.preview_page();
    preview_position_ = word_info.preview_position();
    //TODO: Handle pages_with_relevance_ this would need joining too.
  }

  // Join all pages of both word infos.
  pages_with_relevance_.insert(word_info.pages_with_relevance().begin(), 
      word_info.pages_with_relevance().end());
}
