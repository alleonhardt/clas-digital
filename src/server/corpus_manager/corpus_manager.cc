#include "metadata_handler.h"

///Constructor
CorpusManager CorpusManager(){};

/**
 * Updates all metadata and adds entries to map of entries.
 * @param[in] metadata (json with all metadata)
 */
bool CorpusManager::UpdateZotero(nlohmann::json metadata) {
  
  //Iterate over all items in json
  for (auto &it : metadata) {
      //Create new entry, or overwrite old entry.
      map_entries_[it["key"]] = MetadataHandler(it);

      //If entries's json does not contain the most relevant information, erase 
      if (map_entries_[it["key"]]->CheckJson() == false)
          m_mapBooks.erase(it["key"]);
  }

  return true;
}

std::map<std::string, MetadataHandler>& CorpusManager::map_entries() {
  return map_entries_;
}

