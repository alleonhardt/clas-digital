#define CLASDIGITAL_PLUGIN
#include "plugins/PlugInManager.hpp"
#include "server/server.hpp"

int *globalx;
int test_case = -1;
CLASServer *srv;

extern "C" bool InitialisePlugin(CLASServer *server)
{
  globalx = (int*)server;
  if(*globalx == 0)
  {
    //OK Do test case 0
    *globalx = 1409;
    test_case = 0;
  }
  else if(*globalx == 1)
  {
    test_case = 1;
    return false;
  }
  else
  {
    test_case = 2;
    srv = server;
    srv->GetServerConfig().server_port_ = 10000;
  }
  return true;
}

extern "C" void UnloadPlugin()
{
  if(test_case < 2)
    *globalx = 0; 
}


