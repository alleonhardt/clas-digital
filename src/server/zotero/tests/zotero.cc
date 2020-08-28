#include "zotero/zotero.hpp"
#include <gtest/gtest.h>

TEST(ZoteroReferenceManager, InitialiseFromJSON) { 
  Zotero::ReferenceManager ref;
  
  // Check that the error codes are all on sport when giving flawed jsons to the
  // Reference Manager
  EXPECT_EQ(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }")),Zotero::ReturnCode::OK);


  EXPECT_EQ(ref.Initialise(nlohmann::json::parse("{ \"group_id\": \"ok\" }")),Zotero::ReturnCode::NO_API_KEY);
  

  EXPECT_EQ(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\"}")),Zotero::ReturnCode::NO_GROUP_ID_OR_USER_ID);


  EXPECT_EQ(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\", \"user_id\": \"gro\" }")),Zotero::ReturnCode::USER_ID_AND_GROUP_ID);
}

TEST(ZoteroReferenceManager, InitialiseFromJSONFile) {
  Zotero::ReferenceManager ref;

  EXPECT_EQ(ref.Initialise(std::filesystem::path("whatever.json")),Zotero::ReturnCode::JSON_FILE_DOES_NOT_EXIST);


  std::ofstream ofs("test_file.json",std::ios::out);
  ofs<<"Hallo";
  ofs.close();

  EXPECT_EQ(ref.Initialise(std::filesystem::path("test_file.json")),Zotero::ReturnCode::NOT_A_VALID_JSON);


  ofs.open("test_file.json",std::ios::out);
  ofs<<"{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }";
  ofs.close();

  EXPECT_EQ(ref.Initialise(std::filesystem::path("test_file.json")),Zotero::ReturnCode::OK);

  std::filesystem::remove("test_file.json");
}
