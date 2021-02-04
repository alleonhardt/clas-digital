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
    bool UpdateZotero(clas_digital::IReferenceManager *manager, clas_digital::IFileHandler *handler);

    /**
     * Returns all item references to all items found in the current zotero setup.
     */
    clas_digital::IReferenceManager::ptr_cont_t& item_references();
    clas_digital::IReference* book(std::string ref);

    
    bool CreateWebpage(clas_digital::IReference *item,clas_digital::IFileHandler *handler);
  

  private:
    ///<The item references meaning books in this context.
    clas_digital::IReferenceManager::ptr_cont_t item_references_;
    ///<The collection references of zotero.
    clas_digital::IReferenceManager::ptr_cont_t collection_references_;

    ///<Map of unique authors of all books to create the id for every author
    std::map<std::string,std::vector<std::string>> m_mapUniqueAuthors;
  
};

#endif



