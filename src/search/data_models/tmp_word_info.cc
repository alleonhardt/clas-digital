#include "tmp_word_info.h"
#include <cstddef>

TempWordInfo::TempWordInfo() : pages_with_relevance_(&cmp) {}


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
