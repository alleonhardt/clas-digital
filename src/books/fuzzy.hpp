#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <map>
#include <algorithm>
#include "func.hpp"

namespace fuzzy
{
    /**
    * Levenshtein distance algorithm as a iterative function
    * @parameter const char* (searched word)
    * @parameter const char* (target word)
    * @return int (levenshtein distance)
    **/
    size_t levenshteinDistance(const char* chS,const char* chT);
            
    //Levenshtein Distance algorithm as a recursive function
    int recursiveLD(const char* chS, int len_s, const char* chT, int len_t);

    /**
    * fuzzy_cmp: compare to words with fuzzy search and case insensetive
    * @parameter string (searched word)
    * @parameter string (word)
    * @return bool 
    */
    bool fuzzy_cmp(std::string sWord1, std::string sWord2);

}

#ifdef __DEFINE_ONE__

#endif
