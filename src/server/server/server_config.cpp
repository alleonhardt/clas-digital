#include "server_config.hpp"
#include <fstream>
#include <sstream>

using namespace clas_digital;

Error<ServerConfigReturnCodes> ServerConfig::LoadFromFile(std::filesystem::path configfile)
{
  std::ifstream ifs(configfile.string(),std::ios::in);
  if(!ifs.is_open())
    return Error(ServerConfigReturnCodes::OPEN_CONFIG_FILE,"Cannot open config file at "+configfile.string());

  std::stringstream buffer;
  buffer << ifs.rdbuf();
  ifs.close();
  return LoadFromString(buffer.str());
}

Error<ServerConfigReturnCodes> ServerConfig::LoadFromString(std::string config)
{
  try
  {
    config_ = nlohmann::json::parse(config);

    use_https_ = config_.value("enable_https",false);

    if(use_https_)
    {
      server_port_ = config_.value("port",443);
      cert_ = config_.value("certificate_file","");
      key_ = config_.value("private_key_file","");

      if(cert_.string() == "" || key_.string() == "")
        return Error(ServerConfigReturnCodes::MISSING_CERT_OR_KEY,"No certificate file or no key file specified, but https is enabled!");
    }
    else
      server_port_ = config_.value("port",80);

    if(config_.count("mount_points") > 0)
    {
      for(auto &i : config_["mount_points"])
      {
        std::filesystem::path p = i.get<std::string>();
        if(!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
          return Error(ServerConfigReturnCodes::MOUNT_POINT_DOES_NOT_EXIST,"The mount point does not exists or is no directory. Mount point: "+p.string());
        mount_points_.push_back(p);
      }
    }

    if(config_.count("upload_points") > 0)
    {
      for(auto &i : config_["upload_points"])
      {
        std::filesystem::path p = i.get<std::string>();
        if(!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
          return Error(ServerConfigReturnCodes::UPLOAD_POINT_DOES_NOT_EXIST,"The upload point does not exists or is no directory. Upload point: "+p.string());
        upload_points_.push_back(p);
      }
    }

    if(config_.count("plugins") > 0)
    {
      for(auto &i : config_["plugins"])
      {
        std::filesystem::path p = i.get<std::string>();
        plugins_.push_back(p);
      }
    }

    if(config_.count("refmgr_config") > 0)
    {
      reference_config_ = config_["refmgr_config"];
    }

    reference_manager_ = config_.value("refmgr","zotero");
    debug::log(debug::LOG_WARNING,"No valid reference manager found, using own id system!");
  }
  catch(nlohmann::json::exception &e)
  {
    return Error(ServerConfigReturnCodes::UNKNOWN_ERROR,std::string("Unknown error, message: ")+e.what());
  }

  return Error(ServerConfigReturnCodes::OK);
}
