#include "corpus_manager.h"
#include "reference_management/IReferenceManager.h"

using namespace clas_digital;

bool CorpusManager::UpdateZotero(clas_digital::IReferenceManager *manager) {
  if(!manager)
    return false;

  auto res = manager->GetAllItems(item_references_,clas_digital::IReferenceManager::CacheOptions::CACHE_FORCE_FETCH);

  auto res2 = manager->GetAllCollections(collection_references_,clas_digital::IReferenceManager::CacheOptions::CACHE_FORCE_FETCH);
  auto no_error = res == clas_digital::IReferenceManager::Error::OK
    && res2 == clas_digital::IReferenceManager::Error::OK;

  return no_error;
}

IReferenceManager::ptr_cont_t &CorpusManager::item_references() {
  return item_references_;
}


