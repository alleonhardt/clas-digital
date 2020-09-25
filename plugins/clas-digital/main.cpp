#include "plugins/EventManager.hpp"
#include "plugins/PlugInManager.hpp"
#include "reference_management/IReferenceManager.h"
#include "server.hpp"
#include "server/server.hpp"
#include "server.hpp"

using namespace clas_digital;
std::shared_ptr<PluginServer> serv;


extern "C" bool InitialisePlugin(clas_digital::CLASServer *server)
{
  serv = std::shared_ptr<PluginServer>(new PluginServer(server));
  return true;
}

extern "C" void UnloadPlugin()
{
  serv.reset();
}
