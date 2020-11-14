#include "server/server.hpp"
#include "plugins/PlugInManager.hpp"
#include <catch2/catch.hpp>
#include <debug/debug.hpp>

using namespace clas_digital;

#ifdef _WIN32
const char libpath[] = "bin/test_plugin";
#else
const char libpath[] = "lib/test_plugin";
#endif

/*
TEST_CASE("Load Plugin InitialisePlugin changes int unload changes it again","[PlugInManager]")
{
  PlugInManager manager;
  int x  = 0;
  REQUIRE(manager.LoadPlugin("Test plugin", libpath, (CLASServer*)&x) == true);

  REQUIRE( x == 1409 );

  REQUIRE(manager.UnloadPlugin("Test plugin") == true);

  REQUIRE( x == 0 );
}


TEST_CASE("Load Plugin InitialisePlugin returns false","[PlugInManager]")
{
  PlugInManager manager;
  int x  = 1;
  REQUIRE(manager.LoadPlugin("Test plugin", libpath, (CLASServer*)&x) == false);

  REQUIRE( x == 1 );

  // Cant unload as it does not exist
  REQUIRE(manager.UnloadPlugin("Test plugin") == false);

  // No unload plugin called
  REQUIRE( x == 1 );
}

TEST_CASE("Load Plugin give real server and change config","[PlugInManager]")
{
  PlugInManager manager;
  int x  = 2;
  CLASServer &srv = CLASServer::GetInstance();
  REQUIRE(srv.InitialiseFromString("{}", ":memory:") == false);

  REQUIRE(srv.GetServerConfig()->server_port_ == 80 );
  REQUIRE(manager.LoadPlugin("Test plugin", libpath, &srv) == true);
  REQUIRE(srv.GetServerConfig()->server_port_ == 10000 );

  // Cant unload as it does not exist
  REQUIRE(manager.UnloadPlugin("Test plugin") == true);

  // No unload plugin called
  REQUIRE( x == 2 );
}
*/
