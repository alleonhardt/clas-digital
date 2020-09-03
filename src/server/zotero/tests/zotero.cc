#include <catch2/catch.hpp>
#include "zotero/zotero.hpp"

TEST_CASE("InitialiseFromJSON", "[ZoteroReferenceManager]") { 
  Zotero::ReferenceManager ref;
  
  // Check that the error codes are all on sport when giving flawed jsons to the
  // Reference Manager
  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }")) == Zotero::ReturnCode::OK);


  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"group_id\": \"ok\" }")) == Zotero::ReturnCode::NO_API_KEY);
  

  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\"}")) == Zotero::ReturnCode::NO_GROUP_ID_OR_USER_ID);


  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\", \"user_id\": \"gro\" }")) == Zotero::ReturnCode::USER_ID_AND_GROUP_ID);
}

TEST_CASE("InitialiseFromJSONFile", "[ZoteroReferenceManager]") {
  Zotero::ReferenceManager ref;

  REQUIRE(ref.Initialise(std::filesystem::path("whatever.json")) == Zotero::ReturnCode::JSON_FILE_DOES_NOT_EXIST);


  std::ofstream ofs("test_file.json",std::ios::out);
  ofs<<"Hallo";
  ofs.close();

  REQUIRE(ref.Initialise(std::filesystem::path("test_file.json")) == Zotero::ReturnCode::NOT_A_VALID_JSON);


  ofs.open("test_file.json",std::ios::out);
  ofs<<"{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }";
  ofs.close();

  REQUIRE(ref.Initialise(std::filesystem::path("test_file.json")) == Zotero::ReturnCode::OK);

  std::filesystem::remove("test_file.json");
}
