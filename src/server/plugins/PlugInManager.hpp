#ifndef CLASIDIGITAL_SRC_SERVER_PLUGINS_PLUGINMANAGER_H
#define CLASIDIGITAL_SRC_SERVER_PLUGINS_PLUGINMANAGER_H

#include <vector>
#include <string>
#include <filesystem>
#include <map>
#include "debug/debug.hpp"


class CLASServer;




class PlugInManager
{
  public:
    bool LoadPlugin(std::string alias_name, std::filesystem::path library_path,CLASServer *server);
    bool UnloadPlugin(std::string alias_name);
    ~PlugInManager();

  private:
    typedef bool (*init_plugin)(CLASServer*);
    typedef void (*unload_plugin)();
    std::map<std::string,void*> loaded_plugins_;
};


#endif
