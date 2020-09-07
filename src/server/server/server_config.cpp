#include "server_config.hpp"
#include <fstream>

Error<ServerConfigReturnCodes> ServerConfig::LoadFromFile(std::filesystem::path configfile)
{
  std::string js;
  std::ifstream ifs(configfile.string(),std::ios::in);
  if(ifs.is_open())
    return Error(ServerConfigReturnCodes::OPEN_CONFIG_FILE,"Cannot open config file at "+configfile.string());

  ifs>>js;
  ifs.close();
  return LoadFromString(js);
}

Error<ServerConfigReturnCodes> ServerConfig::LoadFromString(std::string config)
{
  try
  {
    nlohmann::json js = nlohmann::json::parse(config);

    use_https_ = js.value("enable_https",false);

    if(use_https_)
    {
      server_port_ = js.value("port",443);
      cert_ = js.value("certificate_file","");
      key_ = js.value("private_key_file","");

      if(cert_.string() == "" || key_.string() == "")
        return Error(ServerConfigReturnCodes::MISSING_CERT_OR_KEY,"No certificate file or no key file specified, but https is enabled!");
    }
    else
      server_port_ = js.value("port",80);

    if(js.count("mount_points") > 0)
    {
      for(auto &i : js["mount_points"])
      {
        std::filesystem::path p = i.get<std::string>();
        if(!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
          return Error(ServerConfigReturnCodes::MOUNT_POINT_DOES_NOT_EXIST,"The mount point does not exists or is no directory. Mount point: "+p.string());
        mount_points_.push_back(p);
      }
    }

    if(js.count("upload_points") > 0)
    {
      for(auto &i : js["upload_points"])
      {
        std::filesystem::path p = i.get<std::string>();
        if(!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
          return Error(ServerConfigReturnCodes::UPLOAD_POINT_DOES_NOT_EXIST,"The upload point does not exists or is no directory. Upload point: "+p.string());
        upload_points_.push_back(p);
      }
    }
  }
  catch(nlohmann::json::exception &e)
  {
    return Error(ServerConfigReturnCodes::UNKNOWN_ERROR,std::string("Unknown error, message: ")+e.what());
  }

  return Error(ServerConfigReturnCodes::OK);
}
