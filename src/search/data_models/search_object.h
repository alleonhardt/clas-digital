/**
 * @author fux
 * Class to store a search. Always containing the searched word and
 * searchoptions.
 */

#ifndef SRC_SEARCH_DATA_MODELS_SEARCHOBJECT_H_
#define SRC_SEARCH_DATA_MODELS_SEARCHOBJECT_H_

#include "search_options.h"
#include <string>
#include <vector>
class SearchObject {
  public:
    /**
     * constructor
     * @param query (original query)
     * @param search_options describing what search results to filter, how to
     * sort, fuzzyness etc.
     */
    SearchObject(std::string query, SearchOptions search_options);

    // getter:
    std::string query() const;
    const std::vector<std::string>& words() const;
    const std::vector<std::string>& converted_words() const;
    SearchOptions& search_options();
    
  private: 
    const std::string query_; ///< original query.
    std::vector<std::string> words_;	///< seperated word (created in constructor).
    std::vector<std::string> converted_words_;	/// seperated and converted words.
    SearchOptions search_options_;  ///< search_options.
};

#endif

