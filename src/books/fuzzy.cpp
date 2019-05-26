#include "fuzzy.hpp"

namespace fuzzy
{

/**
* Levenshteindistance algorithm as a iterative function
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
* Levenshtein Distance algorithm as a recursive function
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
* fuzzy_cmp: compare to words with fuzzy search and case insensetive
* @parameter string (searched word)
* @parameter string (word)
* @return bool 
*/
bool fuzzy_cmp(std::string sWord1, std::string sWord2)
{
    //Change to lower string
    func::convertToLower(sWord1);
    func::convertToLower(sWord2);

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
    if(len1>len2 && len2/len1 < 0.6)
        return false;
    else if(len1<len2 && len1/len2 < 0.6)
        return false;

    //Calculate levenshtein distance
    size_t ldIterative = levenshteinDistance(sWord1.c_str(), sWord2.c_str());

    //Calculate score
    double score = static_cast<double>(ldIterative)/ std::max(sWord1.length(), sWord2.length());
    double fuzzyness = 0.25;

    //Check whether score is lower than given fuzzyness)
    if(score < fuzzyness && score >= 0)
        return true;

    return false;
}

} //Close namespace
