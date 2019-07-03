#pragma once

#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <map>
#include <list>
#include <locale>
#include <regex>
#include "json.hpp"
#include "src/console/console.hpp"

namespace func 
{

    /*
    * @brief: checks whether a string is in a vector of strings
    * @parameter string
    * @parameter vector<string> 
    * @return bool
    */
    bool in(std::string str, std::vector<std::string> vec);

    /*
    * @brief: checks whether a integer is in a vector of strings
    * @parameter int
    * @parameter vector<int> 
    * @return bool
    */
    template <typename T>
    bool in(T val, std::vector<T> vec) {
        for(unsigned int i=0; i<vec.size(); i++) {
            if(vec[i] == val)
                return true;
        }
        return false;
    }

    /**
    * @brief expects words to be non-capital letters
    * @param[in] chT1 first string to compare
    * @param[in] chT2 second string to compare
    * @return Boolean indicating, whether strings compare or not
    */
    bool compare(const char* chT1, const char* chT2);

    /**
    * @param[in] str1 first string to compare
    * @param[in] str2 string to compare first string with
    * @return Boolean indicating, whether strings compare or not
    */
    bool compare(std::string str1, std::string str2);

    /**
    * @brief expects words to be non-capital letters
    * @param[in] chT1 first string
    * @param[in] chT2 second string, check whether it is in the first
    * @return Boolean indicating, whether string1 contains string2 or not
    */
    bool contains(const char* chT1, const char* chT2);

    /**
    * @brief takes to strings, converts to lower and calls contains (const char*, const char*)
    * @param[in] s1 first string
    * @param[in] s2 second string, check whether it is in the first
    * @return Boolean indicating, whether string1 contains string2 or not
    */
    bool contains(std::string s1, std::string s2);

    /**
    * @param[in, out] str string to be modified
    */
    void convertToLower(std::string &str);

    /** 
    * @brief function checks whether character is a letter with de and fr local
    * @param[in] s char to be checked
    */
    bool isLetter(const char s);

    /**
    * @brief checks whether a string is a word
    * @param[in] chWord string to be checked
    * @return boolean for words/ no word
    */
    bool isWord(const char* chWord);

    /**
    * @param[in] str string to be splitet
    * @param[in] delimitter 
    */
    void split(std::string str, std::string sDelimitter, std::vector<std::string>& vStr);

    /**
    * @param[in] str string to be splitet
    * @param[in] delimitter 
    * @return vector
    */
    std::vector<std::string> split2(std::string str, std::string delimiter);
    
    /**
    * @brief cuts all non-letter-characters from end and beginning of str
    * @param[in, out] string to modify
    */
    void transform(std::string& str);

    /**
    * @param[in] sWords string of which map shall be created 
    * @param[out] mapWords map to which new words will be added
    */
    void extractWordsFromString(std::string sWords, std::map<std::string, int>& mapWords);

    /**
    * @param[in] sWords string of which map shall be created 
    * @param[out] mapWords map to which new words will be added
    */
    void extractWordsFromString(std::string sWords, std::map<std::string, std::vector<size_t>>& mapWords,
                               size_t pageNum);

    /**
    * @brief check whether string indicates, that next page is reached
    * @param[in] buffer 
    * @return 
    */
    bool checkPage(std::string &buffer);

    /**
    * @param[in] sPathToOcr Path to ocr of a book
    * @param[out] mapWords map to which new words will be added
    */
    void extractPages(std::string sPathToOcr, std::map<std::string, std::vector<size_t>>& mapWords);
}


