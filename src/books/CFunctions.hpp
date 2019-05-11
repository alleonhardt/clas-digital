#pragma once

#include <string.h>
#include <string>
#include "json.hpp"

class CFunctions
{
public:

    /**
    * @param[in] chT1 first string to compare
    * @param[in] chT2 second string to compare
    * @return Boolean indicating, whether strings compare or not
    */
    bool compare(const char* chT1, const char* chT2);

    /**
    * @param[out] str remove of spaces from str
    * @return modified string
    */
    std::string removeSpace(std::string str);
    
    /**
    * @param[out] modief to ignore case
    */
    void ignoreCase(std:: string &str);
        
    /**
    * iequals: compare two string and ignore case.
    * @param[in] string a
    * @param[in] string b
    * @return true if strings are equal, false if not
    */
    bool iequals(const char* a, const char* b);

    /**
    * @param[in] sPathToOcr Path to ocr of a book
    * @param[out] mapWords map to which new words will be added
    */
    void createMapOfWords(std::string sPathToOcr, std::map<std::string, int>& mapWords);

    /**
    * @param[in] sWords string of which map shall be created 
    * @param[out] mapWords map to which new words will be added
    */
    void createMapofWordsFromString(std::string sWords, std::map<std::string, int>& mapWords);

    /**
    * @param[in] sPathToWords path to .txt with all words in book
    * @param[out] mapWords map to which new words will be added.
    */
    void loadMapOfWords(std::string sPathToWords, std::map<std::string, int>& mapWords);

};


