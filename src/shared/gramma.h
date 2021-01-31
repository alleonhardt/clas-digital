/**
 * @author: fux
 */

#ifndef SRC_SHARED_GRAMMA_H
#define SRC_SHARED_GRAMMA_H

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "func.hpp"

struct Word {
  std::string basic_form;

  /**
   * possitible attributes are:
   * f = function (subjekt (SUB), Verb (VER) ...
   * c = case (accusativ (ACC), nominativ (NOM) ...
   * n = numerus (pluarl (PLU), singular (SIN)) 
   * s = sex (male (MAS) female (FEM), neutral (NEU)) 
   */
  std::map<std::string, std::vector<std::string>> attributes; 
};

class Dict {
  public:
    Dict(std::string path) {
      std::cout << "Dict: loading json..." << std::endl;
      //Read Json
      std::ifstream read(path);
      if (!read) {
        std::cout << "Problem loading dictionary." << std::endl;
        return;
      }
      nlohmann::json words;
      read >> words;
      read.close();

      std::cout << "Dict: parse list of words and attributes..." << std::endl;
      //Parse list of words and attributes
      for (auto it=words.begin(); it!=words.end(); it++) {
        Word word({it.value()["bf"], {}});
        for (auto jt=it.value()["atts"].begin(); 
            jt!=it.value()["atts"].end(); jt++) {
          for (auto attr : jt.value())
            word.attributes[jt.key()].push_back(attr);
        }
        word_attributes_[it.key()] = word;
      }

      std::cout << "Dict: create base-forms and conjungations..." << std::endl;
      //Create list of basic forms and conjungations
      for (auto it : word_attributes_)
        base_conjugations_[it.second.basic_form].insert(it.first);
    }

    std::string GetBaseForm(std::string word) {
      if (word_attributes_.count(word) > 0)
        return word_attributes_[word].basic_form;
      return "";
    }

    std::set<std::string> GetAllConjugations(std::string word) {
      if (base_conjugations_.count(word) > 0)
        return base_conjugations_[word];
      return std::set<std::string>();
    }

    std::string GetAllPossibleAttributes(std::string word) {
      std::string result = "";
      if(word_attributes_.count(word) == 0) 
        return result;
      for (auto it : word_attributes_[word].attributes) {
        result += it.first + ": ";
        for (auto form : it.second) 
          result += form + ", ";
        result += "\n";
      } 
      return result;
    }

    std::vector<std::string> GetAttribute(std::string word, std::string 
        attribute) {
      if (word_attributes_.count(word) == 0)
        return std::vector<std::string>(); 

      return word_attributes_[word].attributes[attribute];
    }

    bool IsWordX(std::string word, std::string attribute, std::string what) {
      std::vector<std::string> attr_list = GetAttribute(word, attribute);
      if (attr_list.size() == 0)
        return true;
      if (std::find(attr_list.begin(), attr_list.end(), what) != attr_list.end())
        return true;
      return false;
    }  

    bool IsWordX(std::string word, std::string attribute, std::vector
        <std::string> what) {
      std::vector<std::string> attr_list = GetAttribute(word, attribute);
      if (attr_list.size() == 0)
        return true;
      for(auto attr : attr_list) {
        if (std::find(what.begin(), what.end(), attr) != what.end())
          return true;
      }
      return false;
    }
        

  private:
    std::unordered_map<std::string, Word> word_attributes_;
    std::unordered_map<std::string, std::set<std::string>> base_conjugations_;
};

#endif
