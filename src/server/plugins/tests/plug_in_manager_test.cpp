#include "plugins/PlugInManager.hpp"
#include <catch2/catch.hpp>
#include "server/server.hpp"
#include <debug/debug.hpp>

TEST_CASE("Load Plugin InitialisePlugin changes int unload changes it again","[PlugInManager]")
{
  PlugInManager manager;
  int x  = 0;
  REQUIRE(manager.LoadPlugin("Test plugin", "lib/test_plugin", (CLASServer*)&x) == true);

  REQUIRE( x == 1409 );

  REQUIRE(manager.UnloadPlugin("Test plugin") == true);

  REQUIRE( x == 0 );
}


TEST_CASE("Load Plugin InitialisePlugin returns false","[PlugInManager]")
{
  PlugInManager manager;
  int x  = 1;
  REQUIRE(manager.LoadPlugin("Test plugin", "lib/test_plugin", (CLASServer*)&x) == false);

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

  REQUIRE( srv.GetServerConfig().server_port_ == 80 );
  REQUIRE(manager.LoadPlugin("Test plugin", "lib/test_plugin", &srv) == true);
  REQUIRE( srv.GetServerConfig().server_port_ == 10000 );

  // Cant unload as it does not exist
  REQUIRE(manager.UnloadPlugin("Test plugin") == true);

  // No unload plugin called
  REQUIRE( x == 2 );
}
