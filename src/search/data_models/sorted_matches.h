/** 
 * @author fux
 */

#ifndef SRC_SEARCH_DATAMODELS_SORTEDMATCHES_H_
#define SRC_SEARCH_DATAMODELS_SORTEDMATCHES_H_

#include <iostream>
#include <unordered_map>
#include <string>
#include <set>
#include <functional>
#include <utility>
#include <vector>

class SortedMatches {
  private:
    typedef std::pair<std::string, double> weighted_match;
    typedef std::function<bool(const weighted_match&, const weighted_match&)> Cmp;
    std::set<weighted_match, Cmp> matches_;

    static bool cmp(const weighted_match& a, const weighted_match& b) {
      if (a.second == b.second) 
        return a.first > b.first;
      return a.second > b.second;
    }

  public:
    /**
     * Empty constructor.
     */
    SortedMatches();
    /** 
     * Construct adding first match. TODO (fux): check if this is needed.)
     */
    SortedMatches(std::pair<std::string, double> first_match);

    /**
     * Inserts a new match and keeps set sorted. If after insertion more than 10
     * elements exist, the last element is deleted.
     * @param new_match new match to insert.
     */
    void Insert(weighted_match new_match);

    /**
     * Gets first (and best) match.
     * @return best match.
     */
    std::string GetBestMatch();

    /**
     * Gets all matches.
     * @return vector of all matches.
     */
    std::vector<std::string> GetAllMatches() const;
};

#endif
