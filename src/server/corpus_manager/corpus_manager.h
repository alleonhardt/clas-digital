/**
 * @author fux
 */

#ifndef CLASDIGITAL_SRC_SERVER_CORPUSMANAGER_H_
#define CLASDIGITAL_SRC_SERVER_CORPUSMANAGER_H_

#include <iostream>
#include <map>
#include <nlohmann::json>
#include <string>

#include "metadata_handler.h"

/**
 * Loads and stores all data from zotero. Mainly json for metadata.
*/
class CorpusManager {
  public: 

    ///Constructor
    CorpusManager();

    /**
     * Updates all metadata and adds entries to map of entries.
     * @param[in] metadata (json with all metadata)
     */
    UpdateZotero(nlohmann::json metadata);

    std::map<std::string, MetadataHandler>& map_entries();
    
  private:
    std::map<std::string, MetadataHandler> map_entries_;  ///< map of all entries.
};

#endif




