#include "server/server.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Load from string","ServerConfig")
{
  ServerConfig cfg;

  REQUIRE(cfg.LoadFromString("adass").GetErrorCode() == ServerConfigReturnCodes::UNKNOWN_ERROR);


  //No error returned
  REQUIRE(cfg.LoadFromString("{}") == false);

  //Standard port 80
  REQUIRE(cfg.server_port_ == 80);


  //No mount or upload points
  REQUIRE(cfg.mount_points_.size() == 0);
  REQUIRE(cfg.upload_points_.size() == 0);
}


TEST_CASE("Use https","ServerConfig")
{
  ServerConfig cfg;


  nlohmann::json js;
  js["enable_https"] = true;
  REQUIRE(cfg.LoadFromString(js.dump()).GetErrorCode() == ServerConfigReturnCodes::MISSING_CERT_OR_KEY);

  js["certificate_file"] = "./cert";
  js["private_key_file"] = "./key";

  REQUIRE(cfg.LoadFromString(js.dump()).GetErrorCode() == ServerConfigReturnCodes::OK);

  //Standard port 443
  REQUIRE(cfg.server_port_ == 443);


  //No mount or upload points
  REQUIRE(cfg.mount_points_.size() == 0);
  REQUIRE(cfg.upload_points_.size() == 0);

  REQUIRE(cfg.cert_.string() == std::filesystem::path("./cert").string());
  REQUIRE(cfg.key_.string() == std::filesystem::path("./key").string());
}


TEST_CASE("Use multiple mount points","ServerConfig")
{
  ServerConfig cfg;

  {
    nlohmann::json js;
    js["mount_points"].push_back("abc");
    js["mount_points"].push_back("efg");

    REQUIRE(cfg.LoadFromString(js.dump()).GetErrorCode() == ServerConfigReturnCodes::MOUNT_POINT_DOES_NOT_EXIST);
  }


  {
    nlohmann::json js;
    js["mount_points"].push_back("CMakeFiles");

    REQUIRE(cfg.LoadFromString(js.dump()).GetErrorCode() == ServerConfigReturnCodes::OK);
  }

  //Standard port 80
  REQUIRE(cfg.server_port_ == 80);

  //No mount or upload points
  REQUIRE(cfg.mount_points_.size() == 1);
  REQUIRE(cfg.upload_points_.size() == 0);
}
