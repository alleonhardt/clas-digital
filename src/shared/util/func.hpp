#pragma once

#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <map>
#include <list>
#include <clocale>
#include <regex>
#include <set>
#include <codecvt>

namespace func {

  /**
   * Checks whether a given string is in a vector of strings.
   * @param val - values which should be in given array.
   * @param vec - given array.
   * @return boolean indicating, whether val is found in vec.
   */
  bool In(const std::string& str, const std::vector<std::string>& vec);

  /**
   * Checks whether a given value is in a given vector.
   * @param val - values which should be in given array.
   * @param vec - given array.
   * @return boolean indicating, whether val is found in vec.
   */
  template <typename T>
  bool In(T val, std::vector<T> vec) {
    for(unsigned int i=0; i<vec.size(); i++) {
      if(vec[i] == val)
        return true;
    }
    return false;
  }

  /**
   * Compares to strings. Function is case senstitiv.
   * @param[in] chT1 first string to compare
   * @param[in] chT2 second string to compare
   * @return Boolean indicating, whether strings compare or not
   */
  bool Compare(const char* chT1, const char* chT2);

  /**
   * Compares to strings. Function is case in-senstitiv.
   * @param[in] str1 first string to compare
   * @param[in] str2 string to compare first string with
   * @return Boolean indicating, whether strings compare or not
   */
  bool Compare(std::string str1, std::string str2);

  /**
   * Just as `contains`, however the first string needs to begin with the second.
   * Function is case senstitiv.
   * @param[in] str1 - first string
   * @param[in] str2 - second string, check whether it is in the first
   * @return Boolean indicating, whether str1 begins with str2.
   */
  bool ContainsBegin(const char* chT1, const char* chT2);

  /**
  * Takes to strings and check whether the first contains the second string.
  * Function is case senstitiv.
  * @param[in] str1 - first string
  * @param[in] str2 - second string, check whether it is in the first
  * @return Boolean indicating, whether str1 contains str2 or not.
  */
  bool Contains(const char* chT1, const char* chT2);

  /**
  * Takes to strings, converts to lower and calls contains.
  * @param[in] s1 - first string
  * @param[in] s2 - second string, check whether it is in the first
  * @return Boolean indicating, whether string1 contains string2 or not
  */
  bool Contains(std::string s1, std::string s2);

  /**
   * Modify given string, to lower-case representation.
   * @param[out] str - string to be modified.
   */
  void ConvertToLower(std::string &str);

  /**
   * Return giiven string in lower-case representation.
   * @param[in] str - string to convert.
   * @return string in lower-case representation.
   */
  std::string ReturnToLower(const std::string &str);

  /**
   * Escape html code in string. 
   * @param[out] str - string to be modified
   */
  void EscapeHTML(std::string &str);
  
  /**
   * Replaces all non-english (or rather non-utf-8 characters) with the most
   * similar utf-8 character). F.e. 'ä' is converted to 'a'.
   * @param[in] str - string to convert.
   * @return converted string.
   */
  std::string ReplaceMultiByteChars(std::string str);

  /**
   * Cuts all non-letter-characters from end and beginning of str. Uses german
   * locale which also includes most french letters.
   * @param[in, out] string to modify
   */
  void TrimNonLetterChars(std::string& str);

  /**
   * Checks whether a string is a word. Checks number of characters which are
   * digits or chars. Function is taking non-english characters under account.
   * @param[in] word string to be checked
   * @return boolean for word/ no word
   */
  bool IsWord(const char* word);

  /**
   * Split string at given delimitter and fill given array with splitted strings.
   * If the given string contains the delimitter multiple times in a row,
   * then an empty string is added to the array (as expected). To
   * avoid this, filter the array for empty strings.
   * @param[in] str string to be splitet
   * @param[in] delimitter 
   * @param[out] strs - array of string which will be filled.
   */
  void Split(std::string str, std::string sDelimitter, std::vector<std::string>& strs);

  /**
   * Split string at given delimitter and return an array with splitted strings.
   * If the given string contains the delimitter multiple times in a row,
   * then an empty string is added to the array (as expected). To
   * avoid this, filter the array for empty strings.
   * @param[in] str string to be splitet
   * @param[in] delimitter 
   * @return array of splitted strings.
   */
  std::vector<std::string> Split2(std::string str, std::string delimiter);
  
  
  void add_spaces_after(std::string& str, std::vector<char> chars);

  /**
  * @param[in] sWords string of which map shall be created 
  * @param[out] mapWords map to which new words will be added
  */
  std::map<std::string, int> extractWordsFromString(std::string& sWords);

  /**
  * @brief check whether string indicates, that next page is reached
  * @param[in] buffer 
  * @return 
  */
  bool checkPage(std::string &buffer);

  /**
   * Identify word and insert left and write to highlight.
   * Allowing to specify left and write is a more dynamic approach.
   * @param[in, out] str string to modify.
   * @param pos position, where word should be.
   * @param left string to insert left (f.e. "<mark>")
   * @param right string to insert right (f.e. "</mar>")
   */
  void HighlightWordByPos(std::string& str, int pos, std::string left, std::string right);

  /**
   * Trim a string to a max length centering around a given position.
   * @param[in, out] str to trim.
   * @param pos position to center.
   * @param length length to trim string to
   */
  void TrimStringToLength(std::string& str, int pos, int length);

  /**
   * Delete non-valid characters and escape html code.
   * @param[in, out] str string to modify.
   */
  void EscapeDeleteInvalidChars(std::string& str);

  /**
   * Load and return string-content from disc at given path.
   * Returns empty string if path does not exist.
   * @param[in] path to file.
   * @return file-content as string.
   */
  std::string LoadStringFromDisc(std::string path);

  /**
   * Load json from disc.
   * @param path to json which to load.
   * @return loaded json (empy, if path not found or failed to parse)
   */
  nlohmann::json LoadJsonFromDisc(std::string path);

  template<typename T>
  void WriteContentToDisc(std::string path, T content) {
    //Write json to disc.
    try {
      std::ofstream write(path);
      write << content;
      write.close();
    }
    catch (std::exception& e) {
      std::cout << "Failed writing content at path " << path << " to disc: "
        << e.what() << std::endl;
    }
  }

  /**
   * Sort map by value.
   * @param[in] unordered a map which is not yet sorted. 
   * @return sorted set.
   */
	typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;
  typedef std::set<std::pair<std::string, double>, Comp> sorted_set;
  sorted_set SortByRelavance(std::map<std::string, double> unordered);

  /**
   * Converts a json with variable depths and type to (one-level) map, mapping a
   * key (string) to a string.
   * This is done by parsing the given json according to a config json.
   * @param[in] source json.
   * @param[in] config - json describing which values are neccessary and "where
   * to get there.
   * @return map of all needed values.
   */
  std::map<std::string, std::string> ConvertJson(nlohmann::json& source, nlohmann::json& config);

  /**
   * Builds a string from certain tags and values given.
   * Possible keywords might be: 
   * - string: a simple string to add
   * - tag: take a value from a given tag
   * - tag+index: take the element at given position (index) from a tag (which
   *   is expected to be an array).
   * @param json describing how to build string.
   * @param used_fields array to store which tags have been used fro buidling
   * @param fields from which cvertain parts of the string might come from.
   * @return build string.
   */
  std::string ConvertBuild(std::map<std::string, nlohmann::json> build_elems, std::vector<std::string> &used_fields, 
    nlohmann::json &fields);

  /** 
   * Similar to "`build()` however focuessed an joining arrary, but also kinda
   * builds a string from given fields.
   * @param json describing how to build string.
   * @param used_fields array to store which tags have been used fro buidling
   * @param fields from which cvertain parts of the string might come from.
   * @return joined string.
   */
  std::string ConvertJoin(nlohmann::json value, std::vector<std::string>& used_fields, 
    nlohmann::json& fields);

  /** 
   * Extract requested field at given path from a json-object.
   * @param[in] source json.
   * @param path describing where to get the value from.
   * @return json storing a single string, array, boolen or number. (ducktyeing :D)
   */
  nlohmann::json ExtractFieldFromJson(nlohmann::json& source, std::string path);

  /** 
   * Nice function converting a given string keys to byte-representation
   * starting from 2, 4, 8 ... 
   * The original key (name) and a relevance value are stored as value.
   * @param config from which to get keys and relevance.
   * @return map, mapping bit-representation to name-relevance-pair.
   */
  std::map<short, std::pair<std::string, double>> CreateMetadataTags(nlohmann::json search_config);

  /**
   * Get current time as string.
   * @return Return the human readable string representation of the current time.
   */
  std::string GetCurrentTime();
}
