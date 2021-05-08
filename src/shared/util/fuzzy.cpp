#include "fuzzy.hpp"
#include <cstddef>
#include <valarray>

namespace fuzzy
{
  size_t LevenshteinDistance(const char* chS, const char* chT) {
    size_t len_S = strlen(chS)+1;
    size_t len_T = strlen(chT)+1;
    size_t* d = new size_t[len_S*len_T];
    int substitutionCost = 0;

    std::memset(d, 0, sizeof(size_t) * len_S * len_T);

    for(size_t j=0; j<len_T; j++)
      d[j] = j;
    for(size_t i=0; i<len_S; i++)
      d[i*len_T] = i;

    for(size_t j=1, jm=0; j<len_T; j++, jm++) {
      for(size_t i=1, im=0; i<len_S; i++, im++) {
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

  short lshtein(const char* s1, const char* s2, size_t len_s1, size_t len_s2, size_t threshold) {
    // p os the previous and d is the current distance array. dtmp is used for swapping.
    int* p = new int[len_s2 + 1];
    int* d = new int[len_s2 + 1];
    int* dtmp;

    // Fill initial values.
    int MAX = threshold + 1;
    int n=0;
    for (; n<std::min(len_s2+1, threshold+1); ++n)
      p[n] = n;
    std::fill(p+n, p+len_s2+1, MAX);
    std::fill(d, d+len_s2+1, MAX);

    for (int row=1; row < len_s1+1; ++row) {
      char schar = s1[row-1];
      d[0] = row;

      // Set up threshold window
      int x = row - threshold;
      int min = std::max(1, x);
      int max = std::min(len_s2+1, row+threshold+1);

      if (min > 1) 
        d[min-1] = MAX;
      
      for (int col = min; col < max; ++col) {
        if (schar == s2[col-1])
          d[col] = p[col-1];
        else
          d[col] = std::min(p[col-1], std::min(d[col-1], p[col])) + 1;
      }

      // Swap 
      dtmp = p;
      p = d;
      d = dtmp;
    }

    int res = p[len_s2];
    delete[] p;
    delete[] d;
    if (res >= MAX)
      return -1;
    return res;
  }

  short lshteinNoStop(const char* s1, const char* s2, size_t len_s1, size_t len_s2, size_t threshold) {
    // p os the previous and d is the current distance array. dtmp is used for swapping.
    int* p = new int[len_s2 + 1];
    int* d = new int[len_s2 + 1];
    int* dtmp;

    // Fill initial values.
    int MAX = threshold + 1;
    int n=0;
    for (; n<std::min(len_s2+1, threshold+1); ++n)
      p[n] = n;
    std::fill(p+n, p+len_s2+1, MAX);
    std::fill(d, d+len_s2+1, MAX);

    for (int row=1; row < len_s1+1; ++row) {
      char schar = s1[row-1];
      d[0] = row;

      // Set up threshold window
      int x = row - threshold;
      int min = std::max(1, x);
      int max = std::min(len_s2+1, row+threshold+1);

      if (min > 1) 
        d[min-1] = MAX;
      
      for (int col = min; col < max; ++col) {
        if (schar == s2[col-1])
          d[col] = p[col-1];
        else
          d[col] = std::min(p[col-1], std::min(d[col-1], p[col])) + 1;
      }

      // Swap 
      dtmp = p;
      p = d;
      d = dtmp;
    }

    int res = p[len_s2];
    delete[] p;
    delete[] d;
    return res;
  }


  short Contains(const char* input, const char* given, size_t len_input, size_t len_given) {
    // Stop calculating, if input word is longer than given, as f.e. "hundert" can
    // nether be found in "hund"
    if(len_input > len_given) 
      return -1;

    // After if less than length of the searched word is left
    // (i=len_given-len_input), then the word cannot be found in the given
    // string.
    for (size_t i=0; i<=len_given-len_input; i++) {
      bool found = true;
      for (size_t j=0; j<len_input; j++) {
        if (given[i+j] != input[j]) {
          found = false;
          break;
        }
      }
      if (!found)
        continue;
      else if (len_given == len_input)
        return 0;
      // If word found at ending or beginning, return higher score.
      else if (i == 0 || i+len_input == len_given) 
        return 1;
      return 2;
    }
    return -1;
  }

  short cmp(std::string input, std::string given) {
    // Calculate lengths and c_str representations.
    short len_input = input.length();
    short len_given = given.length();
    const char* cinput = input.c_str();
    const char* cgiven = given.c_str();

    //Fast search (full match: 0, beginswith: 0.1, contains: 0.19)
    // size_t fast = Contains(cinput, cgiven, len_input, len_given); 
    // if (fast != -1)
    //   return fast;

    // Calculate threshold in [1,2] depending on length of search word.
    int threshold = (len_input >= 8) ? 2 : 1;

    //Check whether length of words are to far appart.
    if (std::abs(len_input-len_given) > threshold)
      return -1; 
    
    // Calculate levenshtein distance, use the longer word as first word to reduce memory.
    if (len_given > len_input) 
      return lshtein(cgiven, cinput, len_given, len_input, threshold);
    else 
      return lshtein(cinput, cgiven, len_input, len_given, threshold);
  }
} //Close namespace
