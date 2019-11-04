#include "fuzzy.hpp"
using levenshteinSSE::levenshtein;

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

    for(size_t j=0; j<len_T; j++)
        d[j] = j;
    for(size_t i=0; i<len_S; i++)
        d[i*len_T] = i;

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


double fast_search(const char* chS, const char* chIn, size_t lenS, size_t lenIn)
{
    double score = 0;
    bool r = true;
    if(lenS > lenIn) return 1;
    if(lenS != lenIn) {
        score = (static_cast<double>(lenIn) / lenS)/15; 
        if(score > 0.19) score = 0.19;
        else if(score < 0.05) score = 0.05;
    }

    for(size_t i=0; i<strlen(chIn); i++) {
        r = true;
        for(size_t j=0; j<strlen(chS); j++) {
            if(chIn[i+j] != chS[j]) {
                r = false;
                break;
            }
        }
        if(i==0 && r == true) return score;
        else if (r==true) return 0.19;
    }

    return 1;
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
    //Check lengths
    double len1 = sWord1.length();
    double len2 = sWord2.length();

    //Fast search (full match: 0, beginswith: 0.1, contains: 0.19)
    double fast = fast_search(sWord2.c_str(), sWord1.c_str(), len2, len1); 
    if(fast < 1) return fast;


    //Check whether length of words are to far appart.
    if(len1>len2 && len2/len1 < 0.8)      
        return 1;
    else if(len1/len2 < 0.8) 
        return 1; 

    //Calculate levenshtein distance
    //size_t ldIterative = levenshteinDistance(sWord1.c_str(), sWord2.c_str());
    std::basic_string<char> a_(std::begin(sWord1), std::end(sWord1));
    std::basic_string<char> b_(std::begin(sWord2), std::end(sWord2));
    auto distance = levenshtein(a_, b_);

    //Calculate score
    return static_cast<double>(distance)/ len2;
}

} //Close namespace
