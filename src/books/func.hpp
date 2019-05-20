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
    * @param[in, out] str string to be modified
    */
    void convertToLower(std::string &str);

    /**
    * iequals: compare two string and ignore case.
    * @param[in] string a
    * @param[in] string b
    * @return true if strings are equal, false if not
    */
    bool iequals(const char* a, const char* b);

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
    static inline bool checkPage(std::string &buffer);

};


