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

    // **** SETTER **** //
    
    /**
    * @param[in] path set Path to book)
    */
    void setPath(std::string sPath);
    
    /**
    * Create a map of all word of this book
    */
    void createMapWords();

    void safePages();
    void loadPages(std::map<std::string, std::vector<size_t>>& mapWordsPages);

    /*
    * @param[in] sWord searched word
    * @return list of pages on which searched word accures
    */
    std::list<size_t>* getPagesFull(std::string sWord);

    /*
    * @param[in] sWord searched word
    * @return map of pages with vector of words found on this page
    */
    std::map<int, std::vector<std::string>>* getPagesContains(std::string sWord);

    /*
    * @param[in] sWord searched word
    * @return map of pages with vector of words found on this page
    */
    std::map<int, std::vector<std::string>>* getPagesFuzzy(std::string sWord);

    //Create map of pages and found words for i-word (Contains)
    std::map<int, std::vector<std::string>>* findPagesContains(std::string sWord, std::map<std::string, std::vector<size_t>>& mapWordsPages);

    //Create map of pages and found words for i-word (fuzzy)
    std::map<int, std::vector<std::string>>* findPagesFuzzy(std::string sWord, std::map<std::string, std::vector<size_t>>& mapWordsPages);

    //Remove all elements from mapPages, which do not exist in results2. 
    //For all other elements, add the found string from results to on this page to the result
    void removePages(std::map<int, std::vector<std::string>>* mapPages, std::map<int, std::vector<std::string>>* results2);

};
