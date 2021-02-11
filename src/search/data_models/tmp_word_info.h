/**
 * @author: fux.
 */

#ifndef SRC_SEARCH_BOOKMANAGER_WORDINFO_H_
#define SRC_SEARCH_BOOKMANAGER_WORDINFO_H_

#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

class TempWordInfo {

  private: 
    typedef std::pair<size_t, size_t> weighted_match; ///< page:relevance.
    typedef std::function<bool(const weighted_match&, const weighted_match&)> Cmp;
    
    //Membervariable
    std::set<weighted_match, Cmp> pages_with_relevance_; ///< set for sorting + dublicates.
    size_t preview_position_;  ///< Position best preview.
    size_t preview_page_;  ///< Page on which preview is found.
    int relevance_;  ///< Relevance (how often word occures on pages).
    
    static bool cmp(const weighted_match& a, const weighted_match& b) {
      if (a.second == b.second) 
        return a.first > b.first;
      return a.second > b.second;
    }

  public:

    /**
     * Default Constructor.
     */
    TempWordInfo();

    //Getter
    std::set<weighted_match, Cmp>& pages_with_relevance();
    size_t preview_position() const;
    size_t preview_page() const ;
    int relevance() const;

    //Setter
    void set_preview_position(size_t pos);
    void set_preview_page(size_t page);

    //Functions

    /**
     * Add a page to vector of pages using push_back.
     * @param[in] page_with_relevane (page and matching relevance to add)
     */
    void AddPage(weighted_match page_with_relevance);

    /**
     * Adds a new value to relevance: val*(val+1)/2.
     * @param val (new value)
     */
    void IncreaseRelevance(int val);
    
    /**
     * Gets list of pages. Converts set of pages+relevance to list of pages.
     * @return vector of all pages.
     */
    std::vector<size_t> GetAllPages() const;

    /**
     * Gets page with highest relevance (most words on page).
     * @return page with highest relevance.
     */
    size_t GetBestPage();

    /**
     * Join two word-infos.
     * This is mainly used, when after converting a word to lower case, there
     * might be an already existing WordInfo (for the word in upper-case), so we
     * need to joing both WordInfos.
     * @param word_info word info to join with this word info.
     */
    void Join(TempWordInfo& word_info);
};

#endif
