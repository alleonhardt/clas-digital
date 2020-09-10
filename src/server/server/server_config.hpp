#ifndef CLASDIGITAL_SRC_SERVER_SERVER_SERVER_CONFIG_H
#define CLASDIGITAL_SRC_SERVER_SERVER_SERVER_CONFIG_H
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <debug/debug.hpp>

namespace clas_digital
{

  enum class ServerConfigReturnCodes
  {
    OK = 0,
    MISSING_CERT_OR_KEY = 1,
    UNKNOWN_ERROR = 2,
    OPEN_CONFIG_FILE = 3,
    MOUNT_POINT_DOES_NOT_EXIST = 4,
    UPLOAD_POINT_DOES_NOT_EXIST = 5
  };


  using debug::Error;
  struct ServerConfig
  {
    Error<ServerConfigReturnCodes> LoadFromFile(std::filesystem::path configfile);
    Error<ServerConfigReturnCodes> LoadFromString(std::string configfile);


    bool use_https_;
    unsigned short server_port_;
    std::vector<std::filesystem::path> mount_points_;
    std::vector<std::filesystem::path> upload_points_;
    std::vector<std::filesystem::path> plugins_;
    std::filesystem::path cert_;
    std::filesystem::path key_;

    std::string reference_manager_;
    nlohmann::json reference_config_;

    nlohmann::json config_;
    void *additional_data_;
  };
};
#endif
