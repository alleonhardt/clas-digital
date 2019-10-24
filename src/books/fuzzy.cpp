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

            d[i*len_T+j] = std::min (   d[(i-1) * len_T+j    ] + 1,
                           std::min (   d[i     * len_T+(j-1)] + 1,
                                        d[(i-1) * len_T+(j-1)] + substitutionCost));
        }
    }
     
    size_t score = d[len_S*len_T-1];
    delete []d;
    return score; 
}


/**
* @brief compare to words with fuzzy search and case insensetive, AND modify id
* @parameter sWord1 (searched word)
* @parameter sWord2 (word)
* @param[out] ld indicating levenstein (-1 if false, 0 if exact-, 2 if contains-, 1-2 if fuzzy-match)
* @return bool 
*/
double fuzzy_cmp(std::string sWord1, std::string sWord2)
{
    
    /*
    //Check whether length of words are to far appart.
    int diff = sWord1.length()-sWord2.length();
    if(std::abs(diff) > 3)
        return 1;
    */

    if(func::compare(sWord1.c_str(), sWord2.c_str()) == true)
        return 0;

    if(func::containsBegin(sWord2.c_str(), sWord1.c_str()) == true)
        return 0.1;

    if(func::contains(sWord1.c_str(), sWord2.c_str()) == true)
        return 0.19;

    //Calculate Levinstein distance
    double len1 = sWord1.length();
    double len2 = sWord2.length();

    //Check whether length of words are to far appart.
    if(len1>len2 && len2/len1 <= 0.75)
        return 1;
    else if(len1<len2 && len1/len2 <= 0.75)
        return 1; 

    //Calculate levenshtein distance
    size_t ldIterative = levenshteinDistance(sWord1.c_str(), sWord2.c_str());

    //Calculate score
    return static_cast<double>(ldIterative)/ sWord2.length();
}

} //Close namespace
