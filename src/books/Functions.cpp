#include "CFunctions.hpp"

/**
* @param[in] chT1 first string to compare
* @param[in] chT2 second string to compare
* @return Boolean indicating, whether strings compare or not
*/
bool CFunctions::compare(const char* chT1, const char* chT2)
{
    if(strlen(chT1) != strlen(chT2))
        return false;
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        if(chT1[i] != chT2[i])
            return false;
    }
    
    return true;
}

/**
* @param[out] str remove of spaces from str
* @return modified string
*/
std::string CFunctions::removeSpace(std::string str)
{
    for(unsigned int i=0; i<str.length(); i++)
    {
        if(str[i] == ' ')
            str.erase(i, 1);
    }
    return str;
}

/**
* @param[out] modief to ignore case
*/
void CFunctions::ignoreCase(std::string &str)
{
    for(unsigned int i=0; i<str.length(); i++)
    {
        int num = static_cast<int>(str[i]); 
        if(str[i] >= 65 && str[i] <= 90)
            str[i] = (char)num + 32;
    }
}

/**
* iequals: compare two string and ignore case.
* @param[in] string a
* @param[in] string b
* @return true if strings are equal, false if not
*/
bool CFunctions::iequals(const char* chA, const char* chB) 
{
    unsigned int len = strlen(chB);
    if (strlen(chA) != len)
        return false;
    for (unsigned int i = 0; i < len; ++i)
        if (tolower(chA[i]) != tolower(chB[i]))
            return false;
    return true;
}

/**
* @param[in] sPathToOcr Path to ocr of a book
* @param[out] mapWords map to which new words will be added
*/
void CFunctions::createMapOfWords(std::string sPathToOcr, std::map<std::string, int>& mapWords)
{
}

/**
* @param[in] sWords string of which map shall be created 
* @param[out] mapWords map to which new words will be added
*/
void CFunctions::createMapofWordsFromString(std::string sWords, std::map<std::string, int>& mapWords)
{
}

/**
* @param[in] sPathToWords path to .txt with all words in book
* @param[out] mapWords map to which new words will be added.
*/
void CFunctions::loadMapOfWords(std::string sPathToWords, std::map<std::string, int>& mapWords)
{
}


