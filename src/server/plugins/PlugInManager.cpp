#include "PlugInManager.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <libloaderapi.h>
using plugin_handle_type = HMODULE;
#else
#include <dlfcn.h>
using plugin_handle_type = void*;
#endif

using namespace clas_digital;


bool PlugInManager::LoadPlugin(std::string alias_name, std::filesystem::path library_path, CLASServer *server)
{
  plugin_handle_type handle;
#ifdef _WIN32
  if(!std::filesystem::exists(library_path))
  {
    if(library_path.extension() == "")
    {
      library_path.replace_extension(".dll");
    }
  }

  handle = LoadLibraryA(library_path.string().c_str());
  if(handle == NULL)
  {
    debug::log(debug::LOG_ERROR,"Could not open library at path ",library_path.string(),". Extended error code ",GetLastError(),"\n");
    return false;
  }

  auto symbol = GetProcAddress(handle,"InitialisePlugin");
  if(symbol == NULL)
  {
    debug::log(debug::LOG_ERROR,"Could not find function InitialisePlugin in the DLL\n");
    FreeLibrary(handle);
    return false;
  }

  init_plugin init = (init_plugin)symbol;
  bool c = (*init)(server);
  if(!c)
  {
    debug::log(debug::LOG_ERROR,"InitialisePlugin returned false, unloading plugin now. Plugin: ",library_path.string(),"\n");
    FreeLibrary(handle);
    return false;
  }

  loaded_plugins_.insert({alias_name,handle});
#else
  if(!std::filesystem::exists(library_path))
  {
    if(library_path.extension() == "")
    {
#ifdef __APPLE__
      library_path.replace_extension(".dylib");
#else
      library_path.replace_extension(".so");
#endif

      if(!std::filesystem::exists(library_path))
      {
        library_path.replace_filename("lib"+library_path.filename().string());
      }
    }
  }
  handle = dlopen(library_path.string().c_str(),RTLD_NOW);
  if(handle == nullptr)
  {
    debug::log(debug::LOG_ERROR,"Could not open library at path ",library_path.string(),". Extended error code ",dlerror(),"\n");
    return false;
  }

  auto symbol = dlsym(handle,"InitialisePlugin");
  if(symbol == nullptr)
  {
    debug::log(debug::LOG_ERROR,"Could not find function InitialisePlugin in the shared library\n");
    dlclose(handle);
    return false;
  }

  init_plugin init = (init_plugin)symbol;
  bool c = (*init)(server);
  if(!c)
  {
    debug::log(debug::LOG_ERROR,"InitialisePlugin returned false, unloading plugin now. Plugin: ",library_path.string(),"\n");
    dlclose(handle);
    return false;
  }

  loaded_plugins_.insert({alias_name,(void*)handle});
#endif
  return true;
}

bool PlugInManager::UnloadPlugin(std::string alias_name)
{
  try
  {
#ifdef _WIN32
    plugin_handle_type handle = (plugin_handle_type)loaded_plugins_.at(alias_name);
    auto symbol = GetProcAddress(handle,"UnloadPlugin");
    if(symbol != nullptr)
    {
      unload_plugin unload = (unload_plugin)symbol;
      (*unload)();
    }
    FreeLibrary(handle);

#else
    plugin_handle_type handle = loaded_plugins_.at(alias_name);
    auto symbol = dlsym(handle,"UnloadPlugin");
    if(symbol != nullptr)
    {
      unload_plugin unload = (unload_plugin)symbol;
      (*unload)();
    }
    dlclose(handle);
#endif

    if(loaded_plugins_.size() == 1)
      loaded_plugins_.clear();
    else
      loaded_plugins_.erase(alias_name);
  }
  catch(...)
  {
    return false;
  }
  return true;
}



PlugInManager::~PlugInManager()
{
  std::vector<std::string> unload_vec;
  std::for_each(loaded_plugins_.begin(),loaded_plugins_.end(),[&unload_vec](const std::pair<std::string,void*> &pair){unload_vec.push_back(pair.first);});
  std::for_each(unload_vec.begin(),unload_vec.end(),[this](const std::string &pluginName){
      this->UnloadPlugin(pluginName);
      });
}
