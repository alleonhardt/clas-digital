#include "func.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
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

/**
* @brief checks whether a string is in a vector of strings
* @parameter string
* @parameter vector<string> 
* @return bool
*/
bool in(const std::string& str, const std::vector<std::string>& vec)
{
    for(unsigned int i=0; i<vec.size(); i++)
    {
        if(compare(str, vec[i])== true)
            return true;
    }

    return false;
}

/**
* @brief expects words to be non-capital letters
* @param[in] chT1 first string to compare
* @param[in] chT2 second string to compare
* @return Boolean indicating, whether strings compare or not
*/
bool compare(const char* chT1, const char* chT2)
{
    //Return false, if length of words differs
    if(strlen(chT1) != strlen(chT2))
        return false;

    //Iterate over characters. Return false if characters don't match
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        if(chT1[i] != chT2[i])
            return false;
    }
    
    return true;
}


/**
* @brief takes to strings, converts to lower and calls compare (const char*, const char*)
* @param[in] str1 first string to compare
* @param[in] str2 string to compare first string with
* @return Boolean indicating, whether strings compare or not
*/
bool compare(std::string str1, std::string str2)
{
    //Convert both strings to lower
    convertToLower(str1);
    convertToLower(str2);

    //Call normal compare funktion
    return compare(str1.c_str(), str2.c_str());
}

bool containsBegin(const char* chT1, const char* chT2)
{
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        if(chT1[i] != chT2[i])
            return false;
    }

    return true;
}

/**
* @brief expects words to be non-capital letters
* @param[in] chT1 first string
* @param[in] chT2 second string, check whether it is in the first
* @return Boolean indicating, whether string1 contains string2 or not
*/
bool contains(const char* chT1, const char* chT2)
{
    bool r = true;
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        r = true;
        for(unsigned int j=0; j<strlen(chT2); j++)
        {
            if(chT1[i+j] != chT2[j]) {
                r = false;
                break;
            }
        }

        if (r==true)
            return true;
    }

    return false;
}

/**
* @brief takes to strings, converts to lower and calls contains (const char*, const char*)
* @brief expects words to be non-capital letters
* @param[in] s1 first string
* @param[in] s2 second string, check whether it is in the first
* @return Boolean indicating, whether string1 contains string2 or not
*/
bool contains(std::string s1, std::string s2)
{
    //Convert both strings to lower
    convertToLower(s1);
    convertToLower(s2);

    //Call normal contains function
    return contains(s1.c_str(), s2.c_str());
}


/**
* @param[in, out] str string to be modified
*/
void convertToLower(std::string &str)
{
    std::locale loc1("de_DE.UTF8");
    for(unsigned int i=0; i<str.length(); i++)
        str[i] = tolower(str[i], loc1);
}

/**
* @param[in, out] str string to be modified
*/
std::string returnToLower(const std::string &str) {
  std::locale loc1("de_DE.UTF8");
  std::string str2;
  for(unsigned int i=0; i<str.length(); i++)
    str2 += tolower(str[i], loc1);

  return str2;
}

/**
* @param[in, out] str string to be modified
*/
void escapeHTML(std::string &str)
{
    std::map<char, std::string> replacements = {{'>', "&gt"}, {'<', "&lt"}};
    for(unsigned int i=0; i<str.length(); i++)
    {
        if(replacements.count(str[i]) > 0)
        {
            char s = str[i];
            str.erase(i, 1);
            str.insert(i, replacements[s]);
            i+=replacements[s].length();
        }
    }
}

/**
* @brief split a string at given delimiter. Store strings in array.
* @param[in] str string to be splitet
* @param[in] delimitter 
* @param[in, out] vStr vector of slpitted strings
*/
void split(std::string str, std::string delimiter, std::vector<std::string>& vStr)
{
    size_t pos=0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        if(pos!=0)
            vStr.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }

    vStr.push_back(str);
} 

/**
* @brief split string at given delimiter. Return string in array.
* @param[in] str string to be splitet
* @param[in] delimitter 
* @return vector
*/
std::vector<std::string> split2(std::string str, std::string delimiter)
{
    std::vector<std::string> vStr;

    size_t pos=0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        if(pos!=0)
            vStr.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    vStr.push_back(str);

    return vStr;
}

std::string convertStr(std::string str) {
  std::map<wchar_t, wchar_t> rep = {{L'ä','a'},{L'ö','o'},{L'ü','u'},{L'ö','o'},{L'ß','s'},{L'é','e'},{L'è','e'},{L'á','a'},{L'ê','e'},{L'â','a'}, {L'ſ','s'}};
  
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wide = converter.from_bytes(str); 
  for(size_t i=0; i<wide.length(); i++) {
    if(rep.count(wide[i]) > 0) 
      wide[i] = rep[wide[i]];
  }
  std::string newStr = converter.to_bytes(wide);
  return newStr;
}

/**
* @brief cuts all non-letter-characters from end and beginning of str
* @param[in, out] string to modify
*/
void transform(std::string& str) {
  if (str.length() == 0) return;
  std::map<wchar_t, wchar_t> rep = {{L'ä','a'},{L'ö','o'},{L'ü','u'},{L'ö','o'},{L'ß','s'},{L'é','e'},{L'è','e'},{L'á','a'},{L'ê','e'},{L'â','a'}, {L'ſ','s'}};

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wide = converter.from_bytes(str); 
  for(;;) {
    size_t len = wide.length();

    if (len == 0) break;
    // Erase from front in nonsense.
    if (!std::isalpha(wide.front()) && !std::isdigit(wide.front()) && rep.count(wide.front()) == 0) 
      wide.erase(0,1);

    // Erase from back if nonsense.
    if (wide.length() == 0) break;
    if (!std::isalpha(wide.back()) && !std::isdigit(wide.back()) && rep.count(wide.back()) == 0) 
      wide.pop_back();
    
    // if the length did not change, breack.
    if (len == wide.length())
      break;
  }
  str = (wide.length() == 0) ? "" : converter.to_bytes(wide);
}

/**
* @brief checks whether a string is a word
* @param[in] chWord string to be checked
* @return boolean for words/ no word
*/
bool isWord(const char* word) {
  //Variables 
  size_t count_err = 0;
  size_t count_len = 0;
  size_t max = strlen(word);
  int length;
  wchar_t dest;

  //Set locale
  std::locale loc("de_DE.utf8");
  std::setlocale(LC_ALL, "de_DE.utf8");

  mbtowc(NULL, NULL, 0); 

  //Calculate number of non-letter
  while (max > 0) {
    length = mbtowc(&dest, word, max);
    if (length < 1) break;
    if (!isalpha(dest, loc) && !std::isdigit(dest, loc)) count_err++;
    count_len++;
    word += length; 
    max-=length;
  }

  //Calculate whether more the 30% of characters are non-letters (return false)
  if (static_cast<double>(count_err)/static_cast<double>(count_len) <= 0.3)
      return true;

  return false;
}

void add_spaces_after(std::string& str, std::vector<char> chars) {
  for (size_t i=0; i<str.length(); i++) {
    for (const char c : chars) {
      if (str[i] == c) 
        str.insert(++i, " ");
    }
  }
}

/**
* @brief extract words from a string into a map (drop all sequences which  aren't a word).
* @param[in] sWords string of which map shall be created 
* @param[out] mapWords map to which new words will be added
*/
std::map<std::string, int> extractWordsFromString(std::string& buffer) {
  //Replace all \n and ; with " "
  add_spaces_after(buffer, {'.', ',',';',':'}); // assure splitting.

  std::vector<std::string> possible_words = split2(buffer, " ");
  std::map<std::string, int> words;

  for(unsigned int i=0; i<possible_words.size();i++) {
    if (possible_words[i].length() < 2)
      continue;

    std::string cur_word = possible_words[i];
    transform(cur_word);
    if (cur_word.length() >= 2 && cur_word.length() <= 25 && isWord(cur_word.c_str()) == true) {
      words[cur_word] += 1;
    }
  }
  return words;
}

/**
* @brief check whether string indicates, that next page is reached
* @param[in] buffer 
* @return 
*/
bool checkPage(std::string &buffer) {
  const char arr[] = "----- ";
  if(buffer.length()<6)
    return false;

  for(unsigned int i=0; i < 6;i++) {
    if(arr[i]!=buffer[i])
      return false;
  }

  for(unsigned int i = 6; i < buffer.length(); i++) {
    if(buffer[i]==' ')
      return true;
    else if(buffer[i]<48||buffer[i]>57)
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

void TrimString(std::string& str, int pos, int length) {
  int before = pos-75;  
  int after = pos+75;

  // Correct before if below zero and add negative numbers to after.
  if (before < 0) {
    after+=-1*before;
    before = 0;
  }

  // Correct after and subtract exceeding numbers from before.
  if (after > str.length()) {
    before -= after - str.length();
    // If now before is incorrect (below zero), set before to zero.
    if (before < 0) 
      before = 0;
  }
  str = str.substr(before, after-before);
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
        std::string field_key = func::split2(path_elem.substr(pos+1), "=")[0];
        std::string field_value = func::split2(path_elem.substr(pos+1), "=")[1];
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

} //Close namespace 
