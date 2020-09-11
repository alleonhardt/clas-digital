/**
 * @author: fux.
 */

#ifndef SRC_SEARCH_BOOKMANAGER_WORDINFO_H_
#define SRC_SEARCH_BOOKMANAGER_WORDINFO_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

class WordInfo {
  public:
    /**
     * Constructor.
     * @param[in] pages (vector of all pages word occures on)
     * @param[in] pos (first position the word occures in book)
     * @param[in] rel (relevance of this word)
     */
    WordInfo(std::vector<size_t> pages, int pos, size_t rel);

    //Getter 
    std::vector<size_t> pages();
    int position();
    size_t relevance();

    //Setter
    void set_pages(std::vector<size_t> pages);
    void set_position(int pos);
    void set_relevance(size_t rel);

  private: 

    //Membervariable
    std::vector<size_t> pages_; ///< vector of all pages word occures on.
    //std::map<std::strirng, std::vector<size_t>> neighbors_;
    int position_;  ///< First position this word is found on
    size_t relevance_;  ///< Relevance (how often word occures on pages)
};

#endif
