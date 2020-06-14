#include "func.hpp"

namespace func 
{

/**
* @brief checks whether a string is in a vector of strings
* @parameter string
* @parameter vector<string> 
* @return bool
*/
bool in(const std::string& str, const std::vector<std::string>& vec)
{
    for(unsigned int i=0; i<vec.size(); i++)
    {
        if(compare(str, vec[i])== true)
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
bool compare(const char* chT1, const char* chT2)
{
    //Return false, if length of words differs
    if(strlen(chT1) != strlen(chT2))
        return false;

    //Iterate over characters. Return false if characters don't match
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        if(chT1[i] != chT2[i])
            return false;
    }
    
    return true;
}


/**
* @brief takes to strings, converts to lower and calls compare (const char*, const char*)
* @param[in] str1 first string to compare
* @param[in] str2 string to compare first string with
* @return Boolean indicating, whether strings compare or not
*/
bool compare(std::string str1, std::string str2)
{
    //Convert both strings to lower
    convertToLower(str1);
    convertToLower(str2);

    //Call normal compare funktion
    return compare(str1.c_str(), str2.c_str());
}

bool containsBegin(const char* chT1, const char* chT2)
{
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        if(chT1[i] != chT2[i])
            return false;
    }

    return true;
}

/**
* @brief expects words to be non-capital letters
* @param[in] chT1 first string
* @param[in] chT2 second string, check whether it is in the first
* @return Boolean indicating, whether string1 contains string2 or not
*/
bool contains(const char* chT1, const char* chT2)
{
    bool r = true;
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        r = true;
        for(unsigned int j=0; j<strlen(chT2); j++)
        {
            if(chT1[i+j] != chT2[j]) {
                r = false;
                break;
            }
        }

        if (r==true)
            return true;
    }

    return false;
}

/**
* @brief takes to strings, converts to lower and calls contains (const char*, const char*)
* @brief expects words to be non-capital letters
* @param[in] s1 first string
* @param[in] s2 second string, check whether it is in the first
* @return Boolean indicating, whether string1 contains string2 or not
*/
bool contains(std::string s1, std::string s2)
{
    //Convert both strings to lower
    convertToLower(s1);
    convertToLower(s2);

    //Call normal contains function
    return contains(s1.c_str(), s2.c_str());
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
* @param[in, out] str string to be modified
*/
std::string returnToLower(std::string &str)
{
    std::locale loc1("de_DE.UTF8");
    std::string str2;
    for(unsigned int i=0; i<str.length(); i++)
        str2 += tolower(str[i], loc1);

    return str2;
}



/**
* @brief split a string at given delimiter. Store strings in array.
* @param[in] str string to be splitet
* @param[in] delimitter 
* @param[in, out] vStr vector of slpitted strings
*/
void split(std::string str, std::string delimiter, std::vector<std::string>& vStr)
{
    size_t pos=0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        if(pos!=0)
            vStr.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }

    vStr.push_back(str);
} 

/**
* @brief split string at given delimiter. Return string in array.
* @param[in] str string to be splitet
* @param[in] delimitter 
* @return vector
*/
std::vector<std::string> split2(std::string str, std::string delimiter)
{
    std::vector<std::string> vStr;

    size_t pos=0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        if(pos!=0)
            vStr.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    vStr.push_back(str);

    return vStr;
}

std::string convertStr(std::string& str)
{
    std::map<wchar_t, wchar_t> rep = {{L'ä','a'},{L'ö','o'},{L'ü','u'},{L'ö','o'},{L'ß','s'},{L'é','e'},{L'è','e'},{L'á','a'},{L'ê','e'},{L'â','a'},{L'ſ','s'}};
    
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(str); 
    for(size_t i=0; i<wide.length(); i++)
        if(rep.count(wide[i]) > 0) wide[i] = rep[wide[i]];

    std::string newStr= converter.to_bytes(wide);
    return newStr;
}

/**
* @brief cuts all non-letter-characters from end and beginning of str
* @param[in, out] string to modify
*/
void transform(std::string& str)
{
    if(str.length() == 0)
        return;

    if(isalpha(str.front()) == false)
        transform(str.erase(0,1));

    if(str.length() == 0)
        return;

    if(isalpha(str.back()) == false)
    {
        str.pop_back();
        transform(str);
    }
}

/**
* @brief checks whether a string is a word
* @param[in] chWord string to be checked
* @return boolean for words/ no word
*/
bool isWord(const char* chWord) 
{
    //Variables 
    size_t count_err = 0;
    size_t count_len = 0;
    size_t max = strlen(chWord);
    int length;
    wchar_t dest;

    //Set locale
    std::locale loc("de_DE.utf8");
    std::setlocale(LC_ALL, "de_DE.utf8");

    mbtowc (NULL, NULL, 0); 

    //Calculate number of non-letter
    while (max>0) {
        length = mbtowc(&dest, chWord, max);
        if(length<1) break;
        if(isalpha(dest, loc) == false) count_err++;
        count_len++;
        chWord+=length; max-=length;
    }

    //Calculate whether more the 30% of characters are non-letters (return false)
    if(static_cast<double>(count_err)/static_cast<double>(count_len) <= 0.3)
        return true;

    return false;
}

/**
* @brief extract words from a string into a map (drop all sequences which  aren't a word).
* @param[in] sWords string of which map shall be created 
* @param[out] mapWords map to which new words will be added
*/
std::map<std::string, int> extractWordsFromString(std::string& sBuffer)
{
    //Replace all \n and ; with " "
    std::replace(sBuffer.begin(), sBuffer.end(), '\n', ' ');
    std::replace(sBuffer.begin(), sBuffer.end(), ';', ' ');
    std::replace(sBuffer.begin(), sBuffer.end(), ',', ' ');

    std::vector<std::string> vStrs = split2(sBuffer, " ");
    std::map<std::string, int> mapWords;

    for(unsigned int i=0; i<vStrs.size();i++)
    {
        if (vStrs[i].length() <= 2)
            continue;

        transform(vStrs[i]);
        if(vStrs[i].length() >= 2 && vStrs[i].length() <= 25 && isWord(vStrs[i].c_str()) == true)
        {
            vStrs[i].erase(std::remove(vStrs[i].begin(), vStrs[i].end(), '-'), vStrs[i].end());
            vStrs[i].erase(std::remove(vStrs[i].begin(), vStrs[i].end(), '.'), vStrs[i].end());
            sBuffer.erase(std::remove(sBuffer.begin(), sBuffer.end(), '-'), sBuffer.end());
            sBuffer.erase(std::remove(sBuffer.begin(), sBuffer.end(), '.'), sBuffer.end());
            convertToLower(vStrs[i]);

            mapWords[vStrs[i]] += 1;
        }

    }

    return mapWords;
}


/**
* @brief check whether string indicates, that next page is reached
* @param[in] buffer 
* @return 
*/
bool checkPage(std::string &buffer)
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
