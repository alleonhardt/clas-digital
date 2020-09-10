#include <catch2/catch.hpp>
#include "zotero/zotero.hpp"
#include <openssl/x509.h>

using namespace clas_digital;

TEST_CASE("InitialiseFromJSON", "[ZoteroReferenceManager]") { 
  ZoteroReferenceManager ref;
  
  // Check that the error codes are all on sport when giving flawed jsons to the
  // Reference Manager
  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }")) == ReturnCode::OK);


  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"group_id\": \"ok\" }")) == ReturnCode::NO_API_KEY);
  

  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\"}")) == ReturnCode::NO_GROUP_ID_OR_USER_ID);


  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\", \"user_id\": \"gro\" }")) == ReturnCode::USER_ID_AND_GROUP_ID);
}

TEST_CASE("InitialiseFromJSONFile", "[ZoteroReferenceManager]") {
  ZoteroReferenceManager ref;

  // Check the error code when the file does not exist
  REQUIRE(ref.Initialise(std::filesystem::path("whatever.json")) == ReturnCode::JSON_FILE_DOES_NOT_EXIST);


  // Create a test file and put flawed json inside
  std::ofstream ofs("test_file.json",std::ios::out);
  ofs<<"Hallo";
  ofs.close();

  // Check the error code
  REQUIRE(ref.Initialise(std::filesystem::path("test_file.json")) == ReturnCode::NOT_A_VALID_JSON);


  ofs.open("test_file.json",std::ios::out);
  ofs<<"{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }";
  ofs.close();

  // Try a valid json and check the return code
  REQUIRE(ref.Initialise(std::filesystem::path("test_file.json")) == ReturnCode::OK);

  // Clean up the files created
  std::filesystem::remove("test_file.json");
}


TEST_CASE("GetItemMetadata from ReferenceManager","[ZoteroReferenceManager]")
{
  ZoteroReferenceManager ref;
  auto ret = ref.Initialise(std::filesystem::path("zoteroConfig.json"));
  if(ret == ReturnCode::OK)
  {
    auto ptr = ref.GetItemMetadata([](IReference* ref)
        {
          std::cout<<"JSON: "<<ref->json()<<std::endl;
        },"2UB6NTBH",IReferenceManager::CacheOptions::CACHE_USE_CACHED);

    ptr = ref.GetAllCollections([](IReference* ref){std::cout<<ref->GetKey()<<std::endl;},IReferenceManager::CACHE_FORCE_FETCH);
  }
}
