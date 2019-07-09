#include "fuzzy.hpp"

namespace fuzzy
{

/**
* @brief Levenshteindistance algorithm as a iterative function
* @parameter const char* (search string)
* @parameter const char* (target string)
* @return int (levenshteindistance)
**/
size_t levenshteinDistance(const char* chS, const char* chT)
{
    size_t len_S = strlen(chS)+1;
    size_t len_T = strlen(chT)+1;
    size_t* d = new size_t[len_S*len_T];
    int substitutionCost = 0;

    std::memset(d, 0, sizeof(size_t) * len_S * len_T);

    for(size_t j=1, jm=0; j<len_T; j++, jm++)
    {
        for(size_t i=1, im=0; i<len_S; i++, im++)
        {
            if(chS[im] == chT[jm])
                substitutionCost = 0;
            else
                substitutionCost = 1;

            d[i*len_T+j] = std::min (     d[(i-1)*len_T+j  ] + 1,
                            std::min(     d[i*len_T+(j-1)] + 1,
                               d[(i-1)*len_T+(j-1)] + substitutionCost));
        }
    }
     
    size_t score = d[len_S*len_T-1];
    delete []d;
    return score; 
}


/**
* @brief Levenshtein Distance algorithm as a recursive function
* @parameter const char* (searched string)
* @parameter const char* (targeted string)
* @return int (levenshtein distance)
**/
int recursiveLD(const char* chS, int len_s, const char* chT, int len_t)
{
    int cost;

    if(len_s == 0)
        return len_t;
    if(len_t == 0)
        return len_s;

    if(chS[len_s-1] == chT[len_t-1])
        cost = 0;
    else
        cost = 1;

    return std::min(recursiveLD(chS, len_s-1, chT, len_t  ) + 1,
            std::min(recursiveLD(chS, len_s,   chT, len_t-1) + 1,
                     recursiveLD(chS, len_s-1, chT, len_t-1) + cost));
}

/**
* @brief compare to words with fuzzy search and case insensetive
* @parameter string (searched word)
* @parameter string (word)
* @return bool 
*/
bool fuzzy_cmp(std::string sWord1, std::string sWord2)
{
    //If length is 0 skip word
    if(sWord1.length() == 0)
        return false;

    if(func::compare(sWord1.c_str(), sWord2.c_str()) == true)
        return true;
    if(func::contains(sWord1.c_str(), sWord2.c_str()) == true)
        return true;

    
    //Calculate Levinstein distance
    double len1 = sWord1.length();
    double len2 = sWord2.length();

    //Check whether length of words are to far appart.
    if(len1>len2 && len2/len1 <= 0.75)
        return false;
    else if(len1<len2 && len1/len2 <= 0.75)
        return false;

    //Calculate levenshtein distance
    size_t ldIterative = levenshteinDistance(sWord1.c_str(), sWord2.c_str());

    //Calculate score
    double score = static_cast<double>(ldIterative)/ std::max(sWord1.length(), sWord2.length());
    double fuzzyness = 0.21;

    //Check whether score is lower than given fuzzyness)
    if(score < fuzzyness && score >= 0)
        return true;

    return false;
}

/**
* @brief compare to words with fuzzy search and case insensetive, AND modify id
* @parameter sWord1 (searched word)
* @parameter sWord2 (word)
* @param[out] ld indicating levenstein (-1 if false, 0 if exact-, 2 if contains-, 1-2 if fuzzy-match)
* @return bool 
*/
bool fuzzy_cmp(std::string sWord1, std::string sWord2, double& ld)
{
    //If length is 0 skip word
    if(sWord1.length() == 0)
        return false;

    //Calculate Levinstein distance
    double len1 = sWord1.length();
    double len2 = sWord2.length();

    //Check whether length of words are to far appart.
    if(len1>len2 && len2/len1 <= 0.75)
        return false;
    else if(len1<len2 && len1/len2 <= 0.75)
        return false;

    //Calculate levenshtein distance
    size_t ldIterative = levenshteinDistance(sWord1.c_str(), sWord2.c_str());

    //Calculate score
    double score = static_cast<double>(ldIterative)/ std::max(sWord1.length(), sWord2.length());
    double fuzzyness = 0.21;

    if(containsUmlaut(sWord1) == true || containsUmlaut(sWord2) == true)
        fuzzyness = 0.26;

    //Check whether score is lower than given fuzzyness)
    if(score < fuzzyness && score >= 0)
    {
        ld = 1+score;
        return true;
    }

    return false;
}

bool containsUmlaut(std::string&  str)
{
    if(str.find("ä") != std::string::npos)
        return true;
    if(str.find("ö") != std::string::npos)
        return true;
    if(str.find("ü") != std::string::npos)
        return true;

    return false;
}

} //Close namespace
