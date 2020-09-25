/**
 * @author fux, ShadowItaly
 */

#ifndef CLASDIGITAL_SRC_SERVER_CORPUSMANAGER_H_
#define CLASDIGITAL_SRC_SERVER_CORPUSMANAGER_H_

#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include "reference_management/IReferenceManager.h"


/**
 * Loads and stores all data from zotero. Mainly json for metadata.
*/
class CorpusManager {
  public: 
    /**
     * Updates all metadata and adds entries to map of entries.
     * @param[in] manager The connection to the reference management database
     */
    bool UpdateZotero(clas_digital::IReferenceManager *manager);

    clas_digital::IReferenceManager::ptr_cont_t& item_references();
    
  private:
    clas_digital::IReferenceManager::ptr_cont_t item_references_;
    clas_digital::IReferenceManager::ptr_cont_t collection_references_;
};

#endif



