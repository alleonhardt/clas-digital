#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <map>
#include <algorithm>
#include <codecvt>
#include "func.hpp"

namespace fuzzy
{
    /**
    * Levenshtein distance algorithm as a iterative function
    * @parameter const char* (searched word)
    * @parameter const char* (target word)
    * @return int (levenshtein distance)
    **/
    short lshtein(const char* chS, const char* chT, size_t len_s1, size_t len_s2, size_t max_score);
            
    short contains(const char* input, const char* given, size_t len_input, size_t len_given);

    /**
    * @brief compare to words with fuzzy search and case insensetive, AND modify id
    * @parameter sWord1 (searched word)
    * @parameter sWord2 (word)
    * @param[out] ld indicating levenstein (-1 if false, 1 if exact-, 2 if contains-match, 0-fuzzynes)
    * @return bool 
    */
    short cmp(std::string input, std::string given);
}

#ifdef __DEFINE_ONE__

#endif
