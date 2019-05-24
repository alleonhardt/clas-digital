#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <string>
#include <regex>
#include "func.hpp"
#include "fuzzy.hpp"
#include "CMetadata.hpp"

#pragma once 

class CBook
{
private:

    std::string m_sKey;                                 //Key of the book
    std::string m_sPath;                                //Path to book (if exists)
    std::map<std::string, int> m_Words;                 //Map of all words in book.
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
    * @return map of all words in book 
    */
    const std::map<std::string, int>& getMapWords();

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

    /**
    * @brief safe created word list to file
    */
    void safeMapOfWords();

    /*
    * @param[in] sWord searched word
    * @param[in] fuzzyness
    * @return list of pages on which searched word accures
    */
    std::list<int>* getPages(std::string sWord, int fuzzyness);

    /**
    * @brief calls spezifik compare-function depending on fuzzyness
    * @param[in] curWord current word in list of words
    * @param[in] sWord searched word
    * @param[in] fuzzyness 
    * @return bool
    */
    bool compare(std::string curWord, std::string sWord, int fuzzyness);
};
