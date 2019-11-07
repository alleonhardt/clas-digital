#include <iostream>
#include <string>
#include <list>
#include <map>
#include "CBook.hpp"
#include "CSearchOptions.hpp"
#include "func.hpp"
#include "fuzzy.hpp"

#pragma once 

class CSearch
{
private:
    std::string m_sWord;
    CSearchOptions* m_sOpts;
    std::map<std::string, double>* m_mapSR;

    typedef std::unordered_map<std::string, std::map<std::string, double>> MAPWORDS;

public:
    
    /**
    * @brief constructor
    */
    CSearch(CSearchOptions* searchOpts, std::string sWord);
    ~CSearch();

    // *** GETTER *** //


    /**
    * @return searchOptions
    */
    CSearchOptions* getSearchOptions();

    /**
    * @return searched word
    */
    std::string getSearchedWord();


    // *** GETTER (from searchoptions) *** //

    /** 
    * @return fuzzynes
    */
    bool getFuzzyness();

    /** 
    * @return onlyTitle
    */
    int getOnlyTitle();

    /** 
    * @return onlyOcr
    */
    int getOnlyOcr();
    
    // *** SETTER *** //

    /**
    * @brief set searched word.
    * param[in] searchedWord set searched word
    */
    void setWord(std::string sWord);

    /**
    * @brief calls spezific search function, searches, and creates map of  matches. Removes all 
    * books that do not match with search options.
    */
    std::map<std::string, double>* search(MAPWORDS& mWs, MAPWORDS& mWsTitle, MAPWORDS& mWsAuthor,
                std::unordered_map<std::string, CBook*>& mapBooks);

    /**
    * @brief search full-match
    * @param[in] mapWords map of all words with a list of books in which this where accures
    * @param[in, out] mapSR map of search results
    */
    void normalSearch(MAPWORDS& mapWords);

    /**
    * @brief search fuzzy 
    * @param[in] mapWords map of all words with a list of books in which this word accures
    * @param[in, out] mapSR searchresults
    */
    void fuzzySearch(MAPWORDS& mapWords, std::unordered_map<std::string, CBook*>& mapBooks, bool t);

    /*
    * @brief inserts searchResults into map of searchresults and assigns value of match
    * @param[out] mapSR
    * @param[in] found
    * @param[out] matches
    * @param[in] value
    */
    void myInsert(std::map<std::string, double>& found, std::string sMatch, std::unordered_map<std::string, CBook*>& mapBooks, double value);

    /**
    * @brief remove all books that do not match with searchoptions
    * @param[in, out] mapSR map of search results
    */
    void removeBooks(std::unordered_map<std::string, CBook*>& mapBooks);

    /**
    * @brief check whether book-metadata matches with searchoptions
    * @param[in] book to be checked
    * return Boolean
    */
    bool checkSearchOptions(CBook* book);
};
