#include <catch2/catch.hpp>
#include "plugins/EventManager.hpp"
#include "server/server.hpp"
#include "zotero/zotero.hpp"
#include <openssl/x509.h>

using namespace clas_digital;
EventManager evt(&CLASServer::GetInstance());

TEST_CASE("InitialiseFromJSON", "[ZoteroReferenceManager]") { 
  ZoteroReferenceManager ref(&evt);
  
  // Check that the error codes are all on sport when giving flawed jsons to the
  // Reference Manager
  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }")) == IReferenceManager::Error::OK);


  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"group_id\": \"ok\" }")) == IReferenceManager::Error::NO_API_KEY);
  

  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\"}")) == IReferenceManager::Error::NO_GROUP_OR_USER_ID);


  REQUIRE(ref.Initialise(nlohmann::json::parse("{ \"api_key\": \"Hallo\", \"group_id\": \"ok\", \"user_id\": \"gro\" }")) == IReferenceManager::Error::USER_ID_AND_GROUP_ID);
}

TEST_CASE("InitialiseFromJSONFile", "[ZoteroReferenceManager]") {
  ZoteroReferenceManager ref(&evt);

  // Check the error code when the file does not exist
  REQUIRE(ref.Initialise(std::filesystem::path("whatever.json")) == IReferenceManager::Error::JSON_FILE_DOES_NOT_EXIST);


  // Create a test file and put flawed json inside
  std::ofstream ofs("test_file.json",std::ios::out);
  ofs<<"Hallo";
  ofs.close();

  // Check the error code
  REQUIRE(ref.Initialise(std::filesystem::path("test_file.json")) == IReferenceManager::Error::NOT_A_VALID_JSON);


  ofs.open("test_file.json",std::ios::out);
  ofs<<"{ \"api_key\": \"Hallo\", \"group_id\": \"ok\" }";
  ofs.close();

  // Try a valid json and check the return code
  REQUIRE(ref.Initialise(std::filesystem::path("test_file.json")) == IReferenceManager::Error::OK);
  

  // Clean up the files created
  std::filesystem::remove("test_file.json");
}


TEST_CASE("GetItemMetadata from ReferenceManager","[ZoteroReferenceManager]")
{
  EventManager ev(&CLASServer::GetInstance());
  ZoteroReferenceManager ref(&ev);
  auto ret = ref.Initialise(std::filesystem::path("zoteroConfig.json"));
  if(ret == IReferenceManager::Error::OK )
  {
    IReferenceManager::ptr_t item;
    auto ptr = ref.GetItemMetadata(item,"2UB6NTBHsad",IReferenceManager::CacheOptions::CACHE_USE_CACHED);
    REQUIRE(ptr == IReferenceManager::Error::KEY_DOES_NOT_EXIST);

    ptr = ref.GetItemMetadata(item,"2UB6NTBH",IReferenceManager::CacheOptions::CACHE_FORCE_FETCH);
    REQUIRE(ptr == IReferenceManager::Error::OK);
    REQUIRE(item->GetKey() == "2UB6NTBH");

    IReferenceManager::ptr_cont_t coll;
    ptr = ref.GetAllCollections(coll,IReferenceManager::CacheOptions::CACHE_USE_CACHED);
    REQUIRE(ptr == IReferenceManager::Error::OK);
    REQUIRE(coll->size() == 2);
    
    ptr = ref.GetAllCollections(coll,IReferenceManager::CacheOptions::CACHE_FAIL_ON_CACHE_MISS);
    REQUIRE(ptr == IReferenceManager::Error::OK);
    REQUIRE(coll->size() == 2);

    ptr = ref.GetCollectionMetadata(item,coll->begin()->second->GetKey(),IReferenceManager::CacheOptions::CACHE_FAIL_ON_CACHE_MISS);
    REQUIRE(ptr == IReferenceManager::Error::OK);
    REQUIRE(item->GetKey() == coll->begin()->second->GetKey());
  }
}
