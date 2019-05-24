#pragma once

#include <fstream>
#include <string.h>
#include <string>
#include <map>
#include <list>
#include <locale>
#include <regex>
#include "json.hpp"

namespace func 
{

    /*
    * @brief: checks whether a string is in a vector of strings
    * @parameter string
    * @parameter vector<string> 
    * @return bool
    */
    bool in(std::string str, std::vector<std::string> vec);
    
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
    void extractWordsFromString(std::string sWords, std::list<std::string>& mapWords);

    /**
    * @param[in] sPathToOcr Path to ocr of a book
    * @param[out] mapWords map to which new words will be added
    */
    template<typename T>
    void extractWords(std::string sPathToOcr, T& words)
    {
        //Read ocr
        std::ifstream read(sPathToOcr, std::ios::in);

        //Check, whether ocr could be loaded
        if(!read)
            return;

        //Parse through file line by line
        while(!read.eof())
        {
            //Create string for current line
            std::string sBuffer;
            getline(read, sBuffer);

            //Check whether bufffer is empty
            if(sBuffer.length()<=1)
                continue;
            
            //Create words from current line
            extractWordsFromString(sBuffer, words);
         }
    }

    /**
    * @param[in] sPathToWords path to .txt with all words in book
    * @param[out] mapWords map to which new words will be added.
    */
    void loadMapOfWords(std::string sPathToWords, std::map<std::string, int>& mapWords);

    /**
    * @brief check whether string indicates, that next page is reached
    * @param[in] buffer 
    * @return 
    */
    bool checkPage(std::string &buffer);

}


