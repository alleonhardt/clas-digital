#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <string>
#include <regex>
#include "func.hpp"
#include "fuzzy.hpp"
#include "CMetadata.hpp"
#include "src/console/console.hpp"

#pragma once 

class CBook
{
private:

    std::string m_sKey;                                 //Key of the book
    std::string m_sPath;                                //Path to book (if exists)
    bool m_bOcr;                                        //Book has ocr path
    CMetadata m_Metadata;                               //Json with all metadata 

public:

    CBook();

    /**
    * @Brief Constructor
    * @param[in] sPath Path to book
    * @param[in] map map of words in book
    */
    CBook(nlohmann::json jMetadata);


    // **** Getter **** //

    /**
    * @return Key of the book, after extracting it from the path
    */
    const std::string& getKey();

    /**
    * @brief getter function to return the path to the directory of a book
    * @return string (Path to directory of the book)
    */
    const std::string& getPath();

    /**
    * @return Path to directory of the book
    */
    std::string getOcrPath();

    /**
    * @return Boolean, whether book contains ocr or not 
    */
    bool getOcr();

    /**
    * @return info.json of book
    */
    CMetadata& getMetadata();

    /**
    * @return vector with all collections this book is in
    */
    std::vector<std::string> getCollections();

    /**
    * @return lastName, or Name of author
    */
    std::string getAuthor();

    /**
    * @return title of book
    */
    std::string getTitle();

    /**
    * @return date or -1 if date does not exists or is currupted
    */
    int getDate();

    /**
    * @brief return whether book is publically accessable 
    */
    bool getPublic();
 
    // **** SETTER **** //
    
    /**
    * @param[in] path set Path to book)
    */
    void setPath(std::string sPath);
    
    /**
    * @brief Create a map of all word of this book
    */
    void createMapWords();

    /**
    * @brief safe map of all words and pages on which word occures to disc
    */
    void safePages();

    /**
    * @brief load words and pages on which word occures into map
    */
    void loadPages(std::map<std::string, std::vector<size_t>>& mapWordsPages);

    /**
    * @brief getPages calls the matching getPages... function according to fuzzyness
    */
    std::map<int, std::vector<std::string>>* getPages(std::string sInput, int fuzzyness);

    /*
    * @param[in] sWord searched word
    * @return list of pages on which searched word accures
    */
    std::map<int, std::vector<std::string>>* getPagesFull(std::vector<std::string>& vWords);

    /**
    * @brief return map of all pages + words found on page (Contains).
    * @param[in] sWord searched word.
    * @return map of pages with vector of words found on this page.
    */
    std::map<int, std::vector<std::string>>* getPagesContains(std::vector<std::string>& vWords);

    /**
    * @brief return map of all pages + words found on page (fuzzy).
    * @param[in] sWord searched word
    * @return map of pages with vector of words found on this page
    */
    std::map<int, std::vector<std::string>>* getPagesFuzzy(std::vector<std::string>& vWords);

    /**
    * @brief Create map of pages and found words for i-word (Contains).
    * @return map of all pages on which word was found.
    */
    std::map<int, std::vector<std::string>>* findPagesContains(std::string sWord, std::map<std::string, std::vector<size_t>>& mapWordsPages);

    /** 
    * @brief Create map of pages and found words for i-word (fuzzy).
    * @return map of all pages on which word was found.
    */
    std::map<int, std::vector<std::string>>* findPagesFuzzy(std::string sWord, std::map<std::string, std::vector<size_t>>& mapWordsPages);

    /**
    * @brief Remove all elements from mapPages, which do not exist in results2. 
    */
    void removePages(std::map<int, std::vector<std::string>>* mapPages, std::map<int, std::vector<std::string>>* results2);

};
