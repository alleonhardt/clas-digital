#include "plugins/PlugInManager.hpp"
#include "server/server.hpp"

int *globalx;

extern "C" bool InitialisePlugin(CLASServer *server)
{
  globalx = (int*)server;
  *globalx = 1409;
  return true;
}

extern "C" void UnloadPlugin()
{
  *globalx = 0; 
}


