#include "tmp_word_info.h"
#include <cstddef>

TempWordInfo::TempWordInfo() : pages_with_relevance_(&cmp) {}


std::set<TempWordInfo::weighted_match, TempWordInfo::Cmp>& TempWordInfo::pages_with_relevance() {
  return pages_with_relevance_;
}
size_t TempWordInfo::preview_position() const {
  return preview_position_;
}
size_t TempWordInfo::preview_page() const {
  return preview_page_;
}
int TempWordInfo::relevance() const {
  return relevance_;
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

void TempWordInfo::IncreaseRelevance(int val) {
  relevance_ += val*(val+1)/2;
}

size_t TempWordInfo::GetBestPage() {
  return pages_with_relevance_.begin()->first;
}

void TempWordInfo::Join(TempWordInfo& word_info) {
  // Add up relevance of both word-infos.
  relevance_ += word_info.relevance();

  // Find preview_position with better relevance for page.
  size_t page_with_relevance_a = 0;
  for (auto it : pages_with_relevance_) {
    if (it.first == preview_page_) 
      page_with_relevance_a = it.second;
  }
  size_t page_with_relevance_b = 0;
  for (auto it : word_info.pages_with_relevance()) {
    if (it.first == preview_page_) 
      page_with_relevance_b = it.second;
  }
  // Set preview-position and preview-page accordingly.
  if (page_with_relevance_a < page_with_relevance_b) {
    preview_page_ = word_info.preview_page();
    preview_position_ = word_info.preview_position();
  }

  // Join all pages of both word infos.
  pages_with_relevance_.insert(word_info.pages_with_relevance().begin(), 
      word_info.pages_with_relevance().end());
}
