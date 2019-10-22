#include "fuzzy.hpp"

namespace fuzzy
{

/**
* @brief Levenshteindistance algorithm as a iterative function
* @parameter const char* (search string)
* @parameter const char* (target string)
* @return int (levenshteindistance)
**/
size_t levenshteinDistance(const wchar_t* chS, const wchar_t* chT)
{
    size_t len_S = wcslen(chS)+1;
    size_t len_T = wcslen(chT)+1;
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
* @brief compare to words with fuzzy search and case insensetive, AND modify id
* @parameter sWord1 (searched word)
* @parameter sWord2 (word)
* @param[out] ld indicating levenstein (-1 if false, 0 if exact-, 2 if contains-, 1-2 if fuzzy-match)
* @return bool 
*/
double fuzzy_cmp(std::string sWord1, std::string sWord2)
{
    if(sWord1 == sWord2)
        return 0;

    if(sWord1.find(sWord2.c_str()) != std::string::npos)
        return 0.1;

    //Check whether length of words are to far appart.
    if(std::abs(sWord1.length()-sWord2.length()) > 3)
        return 1;

    //Calculate levenshtein distance
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wWord1 = converter.from_bytes(sWord1);
    std::wstring wWord2 = converter.from_bytes(sWord2);
    size_t ldIterative = levenshteinDistance(wWord1.c_str(), wWord2.c_str());

    //Calculate score
    return score = static_cast<double>(ldIterative)/ std::max(wWord1.length(), wWord2.length());
}

} //Close namespace
