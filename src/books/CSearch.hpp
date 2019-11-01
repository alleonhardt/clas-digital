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

public:
    
    /**
    * @brief constructor
    */
    CSearch(CSearchOptions* searchOpts);
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
    std::map<std::string, double>* search(
                std::unordered_map<std::string, std::map<std::string, double>>& mWs, 
                std::unordered_map<std::string, std::map<std::string, double>>& mWsTitle, 
                std::unordered_map<std::string, CBook*>& mapBooks);

    /**
    * @brief search full-match
    * @param[in] mapWords map of all words with a list of books in which this where accures
    * @param[in, out] mapSR map of search results
    */
    void normalSearch(std::unordered_map<std::string, std::map<std::string, double>>& mapWords);

    /**
    * @brief search fuzzy 
    * @param[in] mapWords map of all words with a list of books in which this word accures
    * @param[in, out] mapSR searchresults
    */
    void fuzzySearch(std::unordered_map<std::string, std::map<std::string, double>>& mapWords, 
                                            std::unordered_map<std::string, CBook*>& mapBooks);

    /**
    * @brief check whether searched word matches with author of a book.
    */
    void checkAuthor(std::unordered_map<std::string, CBook*>& mapBooks);

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

    /*
    * @brief inserts searchResults into map of searchresults and assigns value of match
    * @param[out] mapSR
    * @param[in] found
    * @param[out] matches
    * @param[in] value
    */
    void myInsert(std::map<std::string, double>& found, std::string sMatch, std::unordered_map<std::string, CBook*>& mapBooks, double value);

    /**
    * @brief delete searchOptions
    */
    void deleteSearchOptions();

};
