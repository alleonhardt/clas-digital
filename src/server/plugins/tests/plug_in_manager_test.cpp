#include "plugins/PlugInManager.hpp"
#include <catch2/catch.hpp>
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

