#pragma once 

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <regex>
#include "CFunctions.hpp"
#include "CMetadata.hpp"

class CBook
{
private:

    std::string m_sPath;                    //Key of the book
    std::map<std::string, int> m_Words;     //Map of all words in book.
    bool m_bOcr;                            //Book has ocr path
    CMetadata m_Metadata;                  //Json with all metadata 

public:

    CBook ();

    /**
    * @Brief Constructor
    * @param[in] sPath Path to book
    * @param[in] map map of words in book
    */
    CBook(std::string sPath);


    // **** Getter **** //

    /**
    * @brief getter function to return the path to the directory of a book
    * @return string (Path to directory of the book)
    */
    std::string getPath();

    /**
    * @return Path to directory of the book
    */
    std::string getOcrPath();

    /**
    * @return Key of the book, after extracting it from the path
    */
    std::string getKey();
    
    /**
    * @return Boolean, whether book contains ocr or not 
    */
    bool getOcr();

    /**
    * @return map of all words in book 
    */
    std::map<std::string, int>& getMapWords();

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
    * @return date or -1 if date does not exists or is currupted
    */
    int getDate();

    
    // **** Setter **** //

    /**
    * @param[in] bool indicating whether book has ocr or not
    */
    void setOcr(bool bOcr);
    
    /**
    * @param[in] sPath Path to direcory
    */
    void setPath(std::string sPath);


    /**
    * Create a map of all word of this book
    */
    void createMapWords();
};
