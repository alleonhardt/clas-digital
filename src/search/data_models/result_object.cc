
#include "result_object.h"

ResultObject::ResultObject(Book* book, bool found_in_corpus, double init_score) : 
    book_(book), found_in_metadata_(!found_in_corpus), found_in_corpus_(found_in_corpus),
    metadata_score_(0), corpus_score_(1) {
  if (found_in_corpus)
    corpus_score_ = init_score;
  else
    metadata_score_ = init_score;
}

bool ResultObject::found_in_metadata() const {
  return found_in_metadata_;
}
bool ResultObject::found_in_corpus() const {
  return found_in_corpus_;
}
double ResultObject::metadata_score() const {
  return metadata_score_;
}
double ResultObject::corpus_score() const {
  return corpus_score_;
}
Book* ResultObject::book() const {
  return book_;
}

void ResultObject::FoundInMetadataSetInitScore(double init_score) {
  found_in_metadata_ = true;
  metadata_score_ = init_score;
}

void ResultObject::FoundInCorpusSetInitScore(double init_score) {
  found_in_corpus_ = true;
  corpus_score_ = init_score;
}

void ResultObject::IncreaseMetadataScore(double inc) {
  metadata_score_ += inc;
}

void ResultObject::IncreaseCorpusScore(double inc) {
  corpus_score_ += inc;
}

double ResultObject::GetOverallScore() const {
  return metadata_score_ + 10*corpus_score_;
}
