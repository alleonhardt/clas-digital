#pragma once 

#include <fstream>
#include <list>
#include <map>
#include <string>
#include <regex>
#include "CFunctions"
#include "json.hpp"


class CBook
{
private:

    std::string m_sKey;                     //Key of the book
    std::map<std::string, int>* m_Words;    //Map of all words in book.
    bool m_bOcr;                            //Book has ocr path
    nlohmann::json m_jMetadata;              //Json with all metadata 
     

public:

    /**
    * Constructor creates a new book
    * @param string (Path to book)
    * @param map<string, int> (map of words in book)
    */
    CBook(std::string sPath, std::map<std::string, int>* mapWords);

    /**
    * getter function to return the path to the directory of a book
    * @return string (Path to directory of the book)
    */
    std::string getPath() {
        return m_sPath;
    }

    /**
    * getKey: function to extract the key out of the path and return only the key.
    * @return string (key)
    */
    std::string getKey() { 
        return m_sKey;
    }
    
    /**
    * getOcr: return whether book has an ocr or not
    * @return bool
    */
    bool getOcr() {
        return m_bOcr;
    }

    /**
    * getMetadata: get json with metadata of this book
    * @return nlohmann::json (info.json of book)
    */
    nlohmann::json& getJsonMetadata() {
        return m_jMetadata;
    }

    /**
    * getter function to return the path to the ocr.txt file
    * @return string
    */     
    std::string getOcrPath() {
        std::string sPath = m_sPath;
        sPath.append("/ocr.txt");
        return sPath;
    }

    /**
    * getter function to return the path to the ocr.txt file
    * @return string
    */     
    std::string getJsonPath() {
        std::string sPath = m_sPath;
        sPath.append("/info.json");
        return sPath;
    }

    /**
    * getter function to return map of all words in book
    * @return map<string, int> (map of all words in book)
    */
    std::map<std::string, int>* getMapWords() {
        return m_Words;
    }

    /**
    * getter function to return number of letters in book
    * @return int (number of letters in book)
    */
    int getNumLetters () {
        int letters = 0;
        for(auto it = m_Words->begin(); it!=m_Words->end(); it++)
            letters += (*it).first.length();
        return letters;
    }
    
