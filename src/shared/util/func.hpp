#pragma once

#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <map>
#include <list>
#include <clocale>
#include <regex>
#include <codecvt>

namespace func 
{
    /*
    * @brief: checks whether a string is in a vector of strings
    * @parameter string
    * @parameter vector<string> 
    * @return bool
    */
    bool in(const std::string& str, const std::vector<std::string>& vec);

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

    bool containsBegin(const char* chT1, const char* chT2);

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
    * @param[in] str
    */
    std::string returnToLower(std::string &str);

    /**
    * @param[in, out] str string to be modified
    */
    void escapeHTML(std::string &str);
    
    std::string convertStr(std::string &str);

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
    std::map<std::string, int> extractWordsFromString(std::string& sWords);

    /**
    * @brief check whether string indicates, that next page is reached
    * @param[in] buffer 
    * @return 
    */
    bool checkPage(std::string &buffer);
}


