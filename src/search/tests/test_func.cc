#include <catch2/catch.hpp>
#include "func.hpp"


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
