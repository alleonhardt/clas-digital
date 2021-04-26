#include "func.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <exception>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <ostream>
#include <regex>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace func 
{

bool In(const std::string& str, const std::vector<std::string>& vec) {
  for (unsigned int i=0; i<vec.size(); i++) {
    if (Compare(str, vec[i]))
      return true;
  }
  return false;
}

bool Compare(const char* str1, const char* str2) {
  // Return false, if length of words differs.
  if (strlen(str1) != strlen(str2))
    return false;

  // Iterate over characters. Return false if characters don't match.
  for (unsigned int i=0; i<strlen(str1); i++) {
    if (str1[i] != str2[i])
      return false;
  }
  return true;
}

bool Compare(std::string str1, std::string str2) {
  // Convert both strings to lower.
  ConvertToLower(str1);
  ConvertToLower(str2);

  // Call normal compare function.
  return Compare(str1.c_str(), str2.c_str());
}

bool ContainsBegin(const char* str1, const char* str2) {
  for (unsigned int i=0; i<strlen(str1); i++) {
    if (str1[i] != str2[i])
      return false;
  }
  return true;
}

bool Contains(const char* str1, const char* str2) {
  bool found = true;
  for (unsigned int i=0; i<strlen(str1); i++) {
    found = true;
    for (unsigned int j=0; j<strlen(str2); j++) {
      if (str1[i+j] != str2[j]) {
        found = false;
        break;
      }
    }

    if (found)
      return true;
  }
  return false;
}

bool Contains(std::string s1, std::string s2) {
  //Convert both strings to lower
  ConvertToLower(s1);
  ConvertToLower(s2);

  //Call normal contains function
  return Contains(s1.c_str(), s2.c_str());
}

void ConvertToLower(std::string &str) {
  std::locale loc1("de_DE.UTF8");
  for (unsigned int i=0; i<str.length(); i++)
    str[i] = tolower(str[i], loc1);
}

std::string ReturnToLower(const std::string &str) {
  std::locale loc("de_DE.UTF8");
  std::string str2;
  for (unsigned int i=0; i<str.length(); i++)
    str2 += tolower(str[i], loc);

  return str2;
}

void EscapeHTML(std::string &str) {
  std::map<char, std::string> replacements = {{'>', "&gt"}, {'<', "&lt"}};
  for (unsigned int i=0; i<str.length(); i++) {
    if (replacements.count(str[i]) > 0) {
      char s = str[i];
      str.erase(i, 1);
      str.insert(i, replacements[s]);
      i+=replacements[s].length();
    }
  }
}

void Split(std::string str, std::string delimiter, std::vector<std::string>& strs) {
  size_t pos=0;
  while ((pos = str.find(delimiter)) != std::string::npos) {
    if (pos != 0)
      strs.push_back(str.substr(0, pos));
    str.erase(0, pos + delimiter.length());
  }

  strs.push_back(str);
} 

std::vector<std::string> Split2(std::string str, std::string delimiter) {
  std::vector<std::string> vStr;

  size_t pos=0;
  while ((pos = str.find(delimiter)) != std::string::npos) {
    if (pos != 0)
      vStr.push_back(str.substr(0, pos));
    str.erase(0, pos + delimiter.length());
  }
  // Add last element also.
  vStr.push_back(str);
  return vStr;
}

std::string ReplaceMultiByteChars(std::string str) {
  std::map<wchar_t, wchar_t> rep = {{L'ä','a'},{L'ö','o'},{L'ü','u'},{L'ö','o'},
    {L'ß','s'},{L'é','e'},{L'è','e'},{L'á','a'},{L'ê','e'},{L'â','a'}, {L'ſ','s'}};
  
  //Set locale
  std::locale loc("de_DE.utf8");
  std::setlocale(LC_ALL, "de_DE.utf8");

  // Convert given string to wide-char.
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wide = converter.from_bytes(str); 

  // Replace all multiple-byte characters with a matching replacement or a 
  for (size_t i=0; i<wide.length(); i++) {

    char c = static_cast<char>(wide[i]);

    // If current char is found in list of replacements, replace current char.
    if (rep.count(wide[i]) > 0)
      wide[i] = rep[wide[i]];
    
    // Skip all single-byte-characters.
    else if ((int)c >= 0 && (int)c < 256)
      continue;

    // If cur char is non-alpha or -digit, replace this character with '*'.
    else if (!std::isalnum(wide[i], loc))
      wide[i] = L'*';
  }
  std::string newStr = converter.to_bytes(wide);
  return newStr;
}

void TrimNonLetterChars(std::string& str) {
  if (str.length() == 0) return;
  
  //Set locale
  std::locale loc("de_DE.utf8");
  std::setlocale(LC_ALL, "de_DE.utf8");

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wide = converter.from_bytes(str); 
  for (;;) {
    size_t len = wide.length();

    if (len == 0) break;
    // Erase from front in nonsense.
    if (!std::isalnum(wide.front(), loc)) 
      wide.erase(0,1);

    // Erase from back if nonsense.
    if (wide.length() == 0) break;
    if (!std::isalnum(wide.back(), loc))
      wide.pop_back();
    
    // if the length did not change, stop trimming string.
    if (len == wide.length())
      break;
  }
  str = (wide.length() == 0) ? "" : converter.to_bytes(wide);
}

bool IsWord(const char* word) {
  // Variables 
  size_t count_err = 0; // Number of non alpha/ digit characters.
  size_t count_len = 0; // Number of actual characters in the word.
  size_t max = strlen(word); // Number of characters in char representation.
  int length; // Used to calculate the length of the wchar character (1|2).
  wchar_t dest; // Used to store the converted wchar character

  //Set locale
  std::locale loc("de_DE.utf8");
  std::setlocale(LC_ALL, "de_DE.utf8");
  mbtowc(NULL, NULL, 0); 

  //Calculate number of non-letter
  while (max > 0) {
    // Get the wchar representation of the current string, to calculate isalpha/
    // digit correctly responding to the set local. Return the length of the
    // wide character (1 or 2)
    length = mbtowc(&dest, word, max);

    // Check length.
    if (length < 1) 
      break;
    
    // Check, whether string is character or digit, if not increase error-count.
    if (!isalnum(dest, loc))
      count_err++;
   
    // We're counting the actual length as strlen(word) returns 2 for non-utf8
    // characters.
    count_len++; 
    word += length; // Shift next byte, which shall be considered.
    
    // Decrease length by calculated length when converting char to wchar.
    max -= length;
  }

  // Calculate whether more the 30% of characters are non-letters (return false).
  return static_cast<double>(count_err)/static_cast<double>(count_len) <= 0.3;
}

void add_spaces_after(std::string& str, std::vector<char> chars) {
  for (size_t i=0; i<str.length(); i++) {
    for (const char c : chars) {
      if (str[i] == c) 
        str.insert(++i, " ");
    }
  }
}

std::map<std::string, int> extractWordsFromString(std::string& buffer) {
  // Assure splitting, by adding spaces, where they might be missing (often the
  // case when dealing with scanned texts).
  add_spaces_after(buffer, {'.', ',',';',':'}); 

  // Naive attempt to split all words.
  std::vector<std::string> possible_words = Split2(buffer, " ");
  std::map<std::string, int> words;

  // Iterate over all possible words and only add, if they fulfill certain
  // criteria (min, max length, min number of actual characters). Also
  // "transform" word (cut nonsense characters from beginning and end).
  for (unsigned int i=0; i<possible_words.size();i++) {
    // Only check if length is below minimum, as word length might be reduced
    // after next step.
    if (possible_words[i].length() < 2)
      continue;

    // Get current word and erase non-chars or digits at the beginning and ending.
    std::string cur_word = possible_words[i];
    TrimNonLetterChars(cur_word);

    // Only add if word-length criteria or met. Only add if word actually is a word.
    if (cur_word.length() >= 2 && cur_word.length() <= 25 && IsWord(cur_word.c_str())) {
      words[cur_word] += 1; // Increase word-count on this page.
    }
  }
  return words;
}

// ----- 8 -----
bool checkPage(std::string &buffer) {
  const char arr[] = "----- ";
  if (buffer.length()<6)
    return false;

  for (unsigned int i=0; i < 6;i++) {
    if(arr[i]!=buffer[i])
      return false;
  }

  auto found = buffer.rfind(" -----");
  if(found < 7 || found == std::string::npos) {
    return false;
  }

  for (unsigned int i = 6; i < buffer.length(); i++) {
    if (buffer[i] == ' ') {
      return true;
    }
    else if (buffer[i] < 48 || buffer[i] > 57)
      return false;
  }
  return false;
}

void HighlightWordByPos(std::string& str, int pos, std::string left, std::string right) {
  std::set<char> strange_endings = {'"', '.', ',', ';', ':'};
  // Check that position is valid.
  if (pos < 0) {
    std::cout << "\x1B[31mNo preview found!! \033[0m\t\t" << std::endl;
  }
  else {
    // Find end of word (next space, or end of string).
    size_t pos2 = str.find(" ", pos+1);
    if (pos2 == std::string::npos)
      pos2 = str.length();
    
    if (strange_endings.count(str[pos2-1]) > 0)
      pos2--; 

    // Get word and then insert left and right symbol
    std::string word = str.substr(pos, pos2-pos);
    str.replace(pos, pos2-pos, left+word+right);
  }
}

int TrimStringToLength(std::string& str, int pos, int length) {
  int before = pos-75;  
  int after = pos+75;
  int modifications = 0;

  // Correct before if below zero and add negative numbers to after.
  if (before < 0) {
    after+=-1*before;
    modifications += before;
    before = 0;
  }

  // Correct after and subtract exceeding numbers from before.
  if (after > str.length()) {
    before -= after - str.length();
    modifications += after - str.length();
    // If now before is incorrect (below zero), set before to zero.
    if (before < 0) 
      before = 0;
  }
  str = str.substr(before, after-before);
  return modifications;
}

void EscapeDeleteInvalidChars(std::string& str) {
  // Delete invalid chars front.
  for (;;) {
    if ((int)str.front() < 0)
      str.erase(0,1);
    else if ((int)str.back() < 0)
      str.pop_back();
    else
      break;
  }

  //Check vor invalid literals and escape
  for (unsigned int i=str.length(); i>0; i--) {
    if (str[i-1] == '\"' || str[i-1] == '\'' || str[i-1] == '\\')
      str.erase(i-1, 1);
    if (str[i-1] == '<' && str[i] != 'm' && str[i] != '/')
      str.erase(i-1, 1);
  }
}

std::string LoadStringFromDisc(std::string path) {
  // Check if path exists.
  if (!std::filesystem::exists(path)) {
    std::cout << "Path not found: " << path << std::endl;
    return "";
  }

  // Load file from disc and return.
  std::ifstream new_page(path);
  std::string str(
      (std::istreambuf_iterator<char>(new_page)), 
      std::istreambuf_iterator<char>());
  return str;
}

nlohmann::json LoadJsonFromDisc(std::string path) {
  std::string json_content = LoadStringFromDisc(path);
  if (json_content == "")
    return nlohmann::json();
  nlohmann::json json;
  try {
    json = nlohmann::json::parse(json_content);
    return json;
  } catch (std::exception &e) {
    std::cout << "Failed parsing json: " << e.what() << std::endl;
    return nlohmann::json();
  }
}

sorted_set SortByRelavance(std::map<std::string, double> unordered) {
  sorted_set sorted_results(unordered.begin(), unordered.end(), [](const auto &a,const auto &b) {
        if(a.second == b.second) 
          return a.first > b.first;
        return a.second > b.second; } );
  return sorted_results;
}

std::map<std::string, std::string> ConvertJson(nlohmann::json& source, nlohmann::json& config) {
  // Extract fields.
  nlohmann::json extracted_fields;
  for (auto& [key, value]: config["searchableTags"].items()) {

    // Only one tag.
    if (value.contains("tag")) 
      extracted_fields[key] = ExtractFieldFromJson(source, value["tag"]);

    // Take the first found tag (first tag when search is != "").
    else if (value.contains("tags_or")) {
      for (const auto& tag : value["tags_or"]) {
        extracted_fields[key] = ExtractFieldFromJson(source, tag);
        if (extracted_fields[key] != "" && !extracted_fields[key].empty())
          break;
      }
    }

    // Add all tags under found paths.
    else if (value.contains("tags_and")) {
      // Get first tag under first path.
      extracted_fields[key] = ExtractFieldFromJson(source, value["tags_and"][0]);

      // Get tags unter all 1..n paths.
      for (size_t i=1; i<value["tags_and"].size(); i++) {
        nlohmann::json new_value = ExtractFieldFromJson(source, value["tags_and"][i]);
        // If result is an array, add all elements one by one.
        if (new_value.is_array())
          std::for_each(new_value.begin(), new_value.end(), [&, k=key](auto e) { extracted_fields[k].push_back(e); });
        // Otherwise simply add result (this will probably be a string.
        else
          extracted_fields[key] += new_value;
      }
    }
  }

  for (auto& [key, value] : config["regex"].items()) {
    if (!extracted_fields[key].is_number() && !extracted_fields[key].is_string())
      continue;
    std::string cur_val = extracted_fields[key];
    bool found = false;
    for (const auto& it : value) {
      std::regex reg(it);
      std::smatch m;
      if (std::regex_search(cur_val, m, reg)) {
        extracted_fields[key] = m[1];
        found = true;
        break;
      }
    }
    if (!found)
      extracted_fields[key] = "";
  }


  // Handle replacements
  std::map<std::string, std::string> converted_json;
  std::vector<std::string> used_fields;
  for (auto& [key, value] : config["representations"].items()) {
    if (value.contains("join"))
      converted_json[key] = ConvertJoin(value, used_fields, extracted_fields);
    else if (value.contains("build"))
      converted_json[key] = ConvertBuild(value["build"], used_fields, extracted_fields);
  }

  // Add all fields, which have not been used to build replacements and replace null with empty string.
  for (auto& [key, value] : extracted_fields.items()) {
    // check if value is NULL and replace with empty string if so.
    if (value.is_null())
      converted_json[key] = "";
    else if (value.is_array()) {
      std::vector<std::string> vec = value.get<std::vector<std::string>>();
      if (vec.size() == 0)
        converted_json[key] = "";
      else if (vec.size() == 1)
        converted_json[key] = *vec.begin();
      else 
        converted_json[key] = std::accumulate(vec.begin()+1, vec.end(), *vec.begin(), 
          [](const std::string& init, std::string& str) { return init + ";" + str; });
    }
    else
      converted_json[key] = value;
  }

  // Handle regex again (to check is they match on representations).
  for (auto& [key, value] : config["regex"].items()) {
    std::string cur_val = converted_json[key];
    bool found = false;
    for (const auto& it : value) {
      std::regex reg(it);
      std::smatch m;
      if (std::regex_search(converted_json[key], m, reg)) {
        converted_json[key] = m[1];
        found = true;
        break;
      }
    }
    if (!found)
      converted_json[key] = "";
  }

  return converted_json;
}

nlohmann::json ExtractFieldFromJson(nlohmann::json& source, std::string path) {

  nlohmann::json temp = source; 
  size_t pos = 0;
  do {
    pos = path.find("/");
    std::string path_elem = path.substr(0, pos); // Get next element.
    path = path.substr(pos+1);  // Cut string.

    // If array -> elem is array-index, 
    if (temp.is_array()) {
      temp = temp[std::stoi(path_elem)];
    }

    // If object → elem is key.
    else {
      // Might be just "[key]" or in the format: 
      size_t pos = path_elem.find("?"); 
      std::string key = path_elem.substr(0,pos);
      if (!temp.contains(key)) {
        return nlohmann::json();
      }
      temp = temp[key];

      // "[key]?[field_key]=[field_value]" then we're already building the result
      if (pos != std::string::npos) {
        std::string field_key = func::Split2(path_elem.substr(pos+1), "=")[0];
        std::string field_value = func::Split2(path_elem.substr(pos+1), "=")[1];
        nlohmann::json result = nlohmann::json::array();
        for (auto elem : temp) {
          if (elem[field_key] == field_value) {
            nlohmann::json new_value = ExtractFieldFromJson(elem, path);
            if (new_value != NULL && !new_value.empty())  
              result.push_back(new_value);
          }
        }
        return result;
      }
    }
  } while(pos != std::string::npos);

  return temp;
}

std::string ConvertBuild(std::map<std::string, nlohmann::json> build_elems, 
    std::vector<std::string> &used_fields, 
    nlohmann::json &extracted_fields) {

  std::string str = "";
  for (const auto& it : build_elems) {
    std::string new_part = "";
    // Assuming the tag at "field" is an array add the element at "index".
    if (it.first == "get" && extracted_fields.contains(it.second["field"].get<std::string>())) {
      // Skip if not an array.
      if (extracted_fields[it.second["field"].get<std::string>()].is_array()) {
        std::vector<std::string> elems = extracted_fields[it.second["field"].get<std::string>()];
        if (elems.size() > it.second["index"].get<int>())
          new_part += elems[it.second["index"].get<int>()];
      }
    }
    // Simply add given string.
    else if (it.first == "string")
      new_part += it.second;
    // Assuming the element at "tag" is a string, simply add this string.
    else if (it.first == "tag" && extracted_fields.count(it.second.get<std::string>())) {
      if (!extracted_fields[it.second.get<std::string>()].is_null())
        new_part += extracted_fields[it.second.get<std::string>()];
    }
    str += (new_part == "") ? "undefined" : new_part;
  }
  return str;
}

std::string ConvertJoin(nlohmann::json value, std::vector<std::string>& used_fields, 
    nlohmann::json& extracted_fields) {
  // Find length of first list elements (as length of all list elements to
  // join are expected to be of equal length.
  std::string first_tag = value["join"][0];
  size_t n = extracted_fields[first_tag].size();

  // Get separator.
  std::string separator = value["separator"];

  // For for i ∈ {1..n} concat the ith element of all elements in "join" 
  std::string joined_fields = "";
  for (size_t i=0; i<n; i++) {
    for (const std::string& tag : value["join"]) {
      std::vector<std::string> elements = extracted_fields[tag].get<std::vector<std::string>>();
      if (elements.size() > i)
        joined_fields += elements[i] + separator;
    }
  }

  // Add all used tags to used tags and sum-up relevance.
  for (const std::string& tag : value["join"])
    used_fields.push_back(tag);

  // Add all build replacement to replacements and calculate relevance as
  // average-relevance of all used fields.
  return joined_fields.substr(0, joined_fields.length()-separator.length());
}

std::map<short, std::pair<std::string, double>> CreateMetadataTags(nlohmann::json search_config) {

  std::map<short, std::pair<std::string, double>> metadata_tags;
  std::map<std::string, double> tags_relevance;
  
  // Add all "normal" tags and relevance.
  for (const auto& [key, value] : search_config["searchableTags"].items()) {
    tags_relevance[key] = value["relevance"];
  }

  // Add all representations and calculate there relevance.
  for (const auto& [key, value] : search_config["representations"].items()) {
    double relevance = 0;
    int counter = 0;
    if (value.contains("build")) {
      for (const auto& [key, value] : value["build"].items()) {
        if (key == "field" || key == "tag") {
          relevance += tags_relevance[value];
          counter++;
        }
      }
    }
    if (value.contains("join")) {
      for (const auto& tag : value["join"]) {
        relevance += tags_relevance[tag];
        counter++;
      }
    }
    tags_relevance[key] = relevance/ counter;
  }

  // Set relevance of all used tag to zero, so they will be ignored 
  for (const auto& [key, value] : search_config["representations"].items()) {
    for (const auto& tag : value["join"]) {
      if (tags_relevance.count(tag) > 0)
        tags_relevance[tag] = 0;
    }
  }

  // Replace tag with integer representation. 
  size_t i=2;
  for (const auto& it : tags_relevance) {
    metadata_tags[i] = {it.first, it.second}; 
    i*=2;
  }

  return metadata_tags;
}


std::string GetCurrentTime() {
  auto timestamp = std::chrono::system_clock::now();

  std::time_t now_tt = std::chrono::system_clock::to_time_t(timestamp);
  std::tm tm = *std::localtime(&now_tt);

  std::stringstream ss;
  ss << std::put_time(&tm, "%c %Z");
  return ss.str();
}

void pause(std::string msg) {
  std::cout << msg;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
}

} //Close namespace 
