#include "fuzzy.hpp"
#include <cstddef>
#include <valarray>
//using levenshteinSSE::levenshtein;

namespace fuzzy
{

size_t levenshteinDistance(const char* chS, const char* chT) {
  size_t len_S = strlen(chS) + 1;
  size_t len_T = strlen(chT) + 1;
  std::cout << "len_S: " << len_S << std::endl;
  std::cout << "len_T: " << len_T << std::endl;
  size_t* d = new size_t[len_S*len_T];
  int substitutionCost = 0;

  std::memset(d, 0, sizeof(size_t) * len_S * len_T);

  for (size_t j=0; j<len_T; j++)
    d[j] = j;
  for (size_t i=0; i<len_S; i++)
    d[i*len_T] = i;

  size_t counter = 0;
  for (size_t j=1; j<len_T; j++) {

    // Print 
    for (size_t x = 0; x<len_T*len_S; x++) {
      if (x%len_S == 0)
        std::cout << std::endl;
      //std::cout << d[x] << "(" << x << ") "; 
      std::cout << d[x] << " "; 
    }
    std::cout << "\n---------------" << std::endl;
    std::cout << "IMPORTANT (" << counter << "): " << d[counter] << std::endl;

    for (size_t i=1; i<len_S; i++) {
      if (chS[i-1] == chT[j-1])
        substitutionCost = 0;
      else
        substitutionCost = 1;

      d[i*len_T+j] = std::min (   d[(i-1) * len_T+j    ] + 1,
                     std::min (   d[i     * len_T+(j-1)] + 1,
                                  d[(i-1) * len_T+(j-1)] + substitutionCost));
    }
    counter+=len_S+1;
    std::cout << "Second guess: " << d[(len_S-1)*len_T+j] << std::endl;
  }
  
  // Print 
  for (size_t x = 0; x<len_T*len_S; x++) {
    if (x%len_S == 0)
      std::cout << std::endl;
    //std::cout << d[x] << "(" << x << ") "; 
    std::cout << d[x] << " "; 
  }
  std::cout << "\n---------------" << std::endl;
  std::cout << "IMPORTANT (" << counter << "): " << d[counter] << std::endl;
   
  size_t score = d[len_S*len_T-1];
  std::cout << "score: " << score << std::endl;
  delete []d;
  return score; 
}


double fast_search(const char* chS, const char* chIn, size_t lenS, size_t lenIn) {
  double score = 0;
  bool r = true;

  // Stop calculating, if 
  if(lenS > lenIn) 
    return 1;

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

double fuzzy_cmp(std::string sWord1, std::string sWord2) {
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
  size_t distance = levenshteinDistance(sWord1.c_str(), sWord2.c_str());

  //Calculate score
  return static_cast<double>(distance)/ len2;
}

size_t levenshteinDistance2(const char* chS, const char* chT, size_t max_score) {
  size_t len_S = strlen(chS)+1;
  size_t len_T = strlen(chT)+1;
  size_t* d = new size_t[len_S*len_T];
  int substitutionCost = 0;

  std::memset(d, 0, sizeof(size_t) * len_S * len_T);

  for(size_t j=0; j<len_T; j++)
    d[j] = j;
  for(size_t i=0; i<len_S; i++)
    d[i*len_T] = i;

  size_t counter = 0;
  for(size_t j=1, jm=0; j<len_T; j++, jm++) {
    if (d[counter] > max_score) {
      std::cout << "Failed, as " << d[counter] << ">" << max_score << std::endl;
      return 3;
    }
    for(size_t i=1, im=0; i<len_S; i++, im++) {
      if(chS[im] == chT[jm])
        substitutionCost = 0;
      else
        substitutionCost = 1;

      d[i*len_T+j] = std::min (   d[(i-1) * len_T+j    ] + 1,
                     std::min (   d[i     * len_T+(j-1)] + 1,
                                  d[(i-1) * len_T+(j-1)] + substitutionCost));
    }
    counter += len_S+1;
  }
   
  size_t score = d[len_S*len_T-1];
  delete []d;
  return score; 
}


size_t fast_search2(const char* input, const char* given, size_t len_input, size_t len_given) {
  // Stop calculating, if input word is longer than given, as f.e. "hundert" can
  // nether be found in "hund"
  if(len_input > len_given) 
    return 3;

  bool found = true;
  for (size_t i=0; i<len_given; i++) {
    found = true;
    for (size_t j=0; j<len_input; j++) {
      if (given[i+j] != input[j]) {
        found = false;
        break;
      }
    }
    if (!found)
      continue;
    // If word found at ending or beginning, return higher score.
    else if (i==0 || i+len_input == len_given) 
      return 1;
    return 2;
  }

  return 3;
}

size_t fuzzy_cmp2(std::string input, std::string given) {
  // If strings match, return success right alway.
  if (input == given) {
    std::cout << "direct match" << std::endl;
    return 0;
  }
  
  // Calculate lengths
  short len_input = input.length();
  short len_given = given.length();

  //Fast search (full match: 0, beginswith: 0.1, contains: 0.19)
  size_t fast = fast_search2(input.c_str(), given.c_str(), len_input, len_given); 
  if (fast < 3) {
    std::cout << "Fast match: " << fast << std::endl;
    return fast;
  }

  //Check whether length of words are to far appart.
  int max_score = (len_input >= 8) ? 2 : 1;
  if (std::abs(len_input-len_given) > max_score) {
    std::cout << "Failed on length comparison." << std::endl;
    return 3; 
  }

  //Calculate levenshtein distance
  std::cout << "LEVENSTEIN!" << std::endl;
  return levenshteinDistance2(input.c_str(), given.c_str(), max_score);
}

} //Close namespace
