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
     * Default Constructor.
     */
    WordInfo();

    /**
     * Constructor.
     * @param[in] pages (vector of all pages word occures on)
     * @param[in] pos (first position the word occures in book)
     * @param[in] rel (relevance of this word)
     */
    WordInfo(std::vector<size_t> pages, size_t pos, int rel);

    //Getter 
    std::vector<size_t> pages();
    size_t position();
    int relevance();

    //Setter
    void set_pages(std::vector<size_t> pages);
    void set_position(size_t pos);
    void set_relevance(int rel);

    //Functions

    /**
     * Add a page to vector of pages using push_back.
     * @param[in] page (page to add)
     */
    void AddPage(size_t page);

    /**
     * Adds a new value to relevance: val*(val+1)/2.
     * @param val (new value)
     */
    void AddRelevance(int val);

  private: 

    //Membervariable
    std::vector<size_t> pages_; ///< vector of all pages word occures on.
    //std::map<std::strirng, std::vector<size_t>> neighbors_;
    size_t position_;  ///< First position this word is found on
    int relevance_;  ///< Relevance (how often word occures on pages)
};

#endif
