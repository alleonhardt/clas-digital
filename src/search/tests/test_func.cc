#include <algorithm>
#include <catch2/catch.hpp>
#include <cstddef>
#include <iostream>
#include <sys/types.h>
#include "func.hpp"
#include "nlohmann/json.hpp"


TEST_CASE ("Trimming a string works as expected", "[trim_string]") {

 std::string str = "den Handflüglern.\n"
    "Dritte Ordnung der Säuget hier e. \n"
    "Rau bt hier e.  F er a e.\n"
    "Sie haben alle drei Arten von Zähnen und nähren ſich alle mehr oder minder von thieriſchen\n"
    "Subſtanzen,  und der Bau der Backenzähne iſt verſchieden,  je nachdem die Nahrung. \n"
    "Die Füße ſind niemals als Hände zu gebrauchen,  und haben keinen entgegenſetzbaren Daum.\n"
    "Die Säugewarzen ſind in der Zahl verſchieden.\n"
    "Die Einlenkung der Unterkinnlade iſt ſo,  daß oder nur eine ſehr unbedeutende Seitenbewe-\n"
    "gung damit ſtatt hat. \n"
    "Die Augenlöcher ſind von den Schläfengruben nicht getrennt.  Die Jochbogen ſind erhaben und\n"
    "auseinander ſtehend. \n"
    "Der Magen iſt einfach und hautig;  der Darmkanal kurz. \n"
    "Sie nähren ſich von friſchem Fleiſch,  Inſecten;  einige freſſen neben dem Fleiſch\n"
    "auch Vegetabilien,  doch nur Obſt und Wurzeln,  niemals Blätter.\n"
    "Sie ſind über die ganze Erde verbreitet. \n"
    "Sie theilen ſich in mehrere Familien, \n"
    "Er ſt e F am i l i e de r R a ubt hier e. \n"
    "H an dflügler.  Chir op t er a. \n";
 
  SECTION ("Trimming in the middle if the text") {
    std::string cur = str;
    func::TrimString(cur, 100, 150); // str.substr(150-75..150+75])
    std::cout << cur << std::endl;
    REQUIRE(cur[0] == str[25]); // As 150-75 = 25 
    REQUIRE(cur[149] == str[174]); // As 100 + 75 = 175
    REQUIRE(cur.length() == 150);
  }

  SECTION ("Trimming at the front of the text") {
    std::string cur = str;
    func::TrimString(cur, 0, 150); // str.substr(150-75..150+75])
    std::cout << cur << std::endl;
    REQUIRE(cur[0] == str[0]); // As 150-75 = 25 
    REQUIRE(cur[149] == str[149]); // As 100 + 75 = 175
    REQUIRE(cur.length() == 150);
  }
  SECTION ("Trimming at the back of the text") {
    std::string cur = str;
    func::TrimString(cur, str.length(), 150); // str.substr(150-75..150+75])
    std::cout << cur << std::endl;
    REQUIRE(cur[0] == str[str.length()-150]); // As 150-75 = 25 
    REQUIRE(cur[149] == str.back()); // As 100 + 75 = 175
    REQUIRE(cur.length() == 150);
  }
}

TEST_CASE("Converting a json is working", "[convert_json]") {
  nlohmann::json example_case = func::LoadJsonFromDisc("src/search/tests/example_data/convert_json/example1.json");
  nlohmann::json config = example_case["config"];
  nlohmann::json metadata = example_case["metadata"];
  nlohmann::json expected = example_case["expected"];

  std::map<std::string, std::string> new_metadata = func::ConvertJson(metadata, config);

  for (const auto& [key, value] : expected.items()) {
    REQUIRE(new_metadata.count(key) > 0);
    REQUIRE(new_metadata[key] == value["value"]);
    std::cout << key << ": " << new_metadata[key] << std::endl;
  }
}

TEST_CASE("Converting a config to bit representation", "[bit_rep]") {
  nlohmann::json example_case = func::LoadJsonFromDisc("src/search/tests/example_data/convert_json/example1.json");
  nlohmann::json config = example_case["config"];
  nlohmann::json expected = example_case["expected"];

  auto convert_map = func::CreateMetadataTags(config);
  REQUIRE(convert_map.size() == config["searchableTags"].size() + config["representations"].size());
  
  // Assert that for each expected tag a tag exists in the created map (We need
  // to us `std::find_if` instead of `map::count()` as the key of the newly
  // created map ist the unique bit representation and the "name" is only stored
  // in its value.
  for (const auto& it : expected.items()) {
    REQUIRE(std::find_if(convert_map.begin(), convert_map.end(), 
          [&](const auto& tag) { return tag.second.first == it.key(); }) != convert_map.end());
  }
}

TEST_CASE("Using regex when converting json", "[regex]") {
  nlohmann::json example_case = func::LoadJsonFromDisc("src/search/tests/example_data/convert_json/example2.json");
  nlohmann::json config = example_case["config"];
  nlohmann::json metadata = example_case["metadata"];

  std::vector<std::map<std::string, std::string>> items;
  for (auto it : metadata) {
    auto new_metadata = func::ConvertJson(it, config);
    std::cout << "CHECKING " << new_metadata["date"] << " == 2017" << std::endl;
    REQUIRE(new_metadata["date"] == "2017");
    items.push_back(new_metadata);
  }

  size_t counter = 0;
  for (auto it : metadata) {
    std::cout << it["data"]["date"].get<std::string>() << " → " << items[counter++]["date"] << std::endl;
  }

}
