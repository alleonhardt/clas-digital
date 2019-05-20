#include "func.hpp"

namespace func 
{

/**
* @param[in] chT1 first string to compare
* @param[in] chT2 second string to compare
* @return Boolean indicating, whether strings compare or not
*/
bool compare(const char* chT1, const char* chT2)
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
std::string removeSpace(std::string str)
{
    for(unsigned int i=0; i<str.length(); i++)
    {
        if(str[i] == ' ')
            str.erase(i, 1);
    }
    return str;
}

/**
* @param[in, out] str string to be modified
*/
void convertToLower(std::string &str)
{
    std::locale loc1("de_DE.UTF8");
    for(unsigned int i=0; i<str.length(); i++)
        str[i] = tolower(str[i], loc1);
}

/**
* iequals: compare two string and ignore case.
* @param[in] string a
* @param[in] string b
* @return true if strings are equal, false if not
*/
bool iequals(const char* chA, const char* chB) 
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
* @brief function checks whether character is a letter with de and fr local
* @param[in] s char to be checked
*/
bool isLetter(const char s)
{
    std::locale loc1("de_DE.UTF8");
    //std::locale loc2("fr_FR");
    if(std::isalpha(s, loc1) == true) 
        return true;
    else
        return false;
}

/**
* @brief checks whether a string is a word
* @param[in] chWord string to be checked
* @return boolean for words/ no word
*/
bool isWord(const char* chWord) 
{
    unsigned int counter = 0;
    for(unsigned int i=0; i<strlen(chWord); i++)
    {
        if(isLetter(chWord[i]) == false)
            counter++;
    }

    if(static_cast<double>(counter)/static_cast<double>(strlen(chWord)) <= 0.3)
        return true;

    return false;
}

/**
* @param[in] str string to be splitet
* @param[in] delimitter 
*/
void split(std::string str, std::string delimiter, std::vector<std::string>& vStr)
{
    size_t pos=0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        vStr.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }

    vStr.push_back(str);
} 

/**
* @brief cuts all non-letter-characters from end and beginning of str
* @param[in, out] string to modify
*/
void transform(std::string& str)
{
    if(str.length() == 0)
        return;

    if(isLetter(str.front()) == false)
        transform(str.erase(0,1));

    if(str.length() == 0)
        return;

    if(isLetter(str.back()) == false)
    {
        str.pop_back();
        transform(str);
    }
}

/**
* @param[in] sWords string of which map shall be created 
* @param[out] mapWords map to which new words will be added
*/
void extractWordsFromString(std::string sWords, std::map<std::string, int>& mapWords)
{
    std::vector<std::string> vStrs;
    split(sWords, " ", vStrs);

    for(unsigned int i=0; i<vStrs.size(); i++)
    {
        transform(vStrs[i]);
        if(isWord(vStrs[i].c_str()) == true)
        {
            convertToLower(vStrs[i]);
            mapWords[vStrs[i]] = 0;
        }
    }
}

/**
* @param[in] sWords string of which map shall be created 
* @param[out] mapWords map to which new words will be added
*/
void extractWordsFromString(std::string sWords, std::list<std::string>& listWords)
{
    if (checkPage(sWords) == true)
    {
        listWords.push_back("###page###");
        return;
    }

    std::vector<std::string> vStrs;
    split(sWords, " ", vStrs);

    for(unsigned int i=0; i<vStrs.size(); i++)
    {
        transform(vStrs[i]);
        if(isWord(vStrs[i].c_str()) == true)
        {
            convertToLower(vStrs[i]);
            listWords.push_back(vStrs[i]);
        }
    }
}

/**
* @param[in] sPathToWords path to .txt with all words in book
* @param[out] mapWords map to which new words will be added.
*/
void loadMapOfWords(std::string sPathToWords, std::map<std::string, int>& mapWords)
{
    std::ifstream read(sPathToWords);

    std::string sBuffer;
    while(!read.eof())
    {
        //Read new line
        getline(read, sBuffer);

        //Add word to map of words 
        mapWords[sBuffer] = 0;
    }
 
}

/**
* @brief check whether string indicates, that next page is reached
* @param[in] buffer 
* @return 
*/
static inline bool checkPage(std::string &buffer)
{
    const char arr[] = "----- ";
    if(buffer.length()<6)
        return false;

    for(unsigned int i=0; i < 6;i++)
    {
        if(arr[i]!=buffer[i])
        return false;
    }

    for(unsigned int i = 6; i < buffer.length(); i++)
    {
        if(buffer[i]==' ')
            return true;
        else if(buffer[i]<48||buffer[i]>57)
            return false;
   }
   return false;
}


} //Close namespace 
