/**
 * author=fux
 * Class to specify search options.
 */

#ifndef SRC_SEARCH_DATA_MODELS_SEARCHOPTIONS_H_
#define SRC_SEARCH_DATA_MODELS_SEARCHOPTIONS_H_

#include <iostream>
#include <vector>
#include <string>

class SearchOptions {
  public:
    /**
     * Constructor for search-options.
     * No conversions taking place. Only the author name is converted to
     * lower-case.
     * @param fuzzy_search search fuzzy (levensthein + search-contains).
     * @param only_metadata search only in metadata.
     * @param only_corpus search only the corpus.
     * @param year_from	year (ommits all reults before this year).
     * @param year_to	year (ommits all reults after this year).
     * @param sort_result_by sort by: 0=relevance, 1=chronologically, 2=alphabetically
     * @param author last-name of author (converted to lower-case)
     * @param collections list of collections in which to search.
     */
    SearchOptions(
        bool fuzzy_search, 
        bool only_metadata, 
        bool only_corpus,
        int year_from,
        int year_to,
        int sort_results_by,
        std::string author,
        std::vector<std::string> collections);

  // getter:
  bool fuzzy_search() const;
  bool only_metadata() const;
  bool only_corpus() const;
  int year_from() const;
  int year_to() const;
  int sort_results_by() const;
  std::string author() const;
  std::vector<std::string> collections() const;

  private:
    const bool fuzzy_search_; ///< search fuzzy (levensthein + search-contains).
    const bool only_metadata_; ///< search only in metadata.
    const bool only_corpus_; ///< search only the corpus.
    const int year_from_;	///< year (ommits all reults before this year).
    const int year_to_;	///< year (ommits all reults after this year).
    const int sort_result_by_; ///< sort by: 0=relevance, 1=chronologically, 2=alphabetically
    const std::string author_; ///< last-name of author.
    const std::vector<std::string> collections_; ///< list of collections in which to search.
};

#endif
