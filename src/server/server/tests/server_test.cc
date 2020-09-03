#include "server/server.hpp"
#include <catch2/catch.hpp>
#include <thread>
#include <chrono>

TEST_CASE("Constructor", "[CLASServer]") {
  // Check the singleton template works
  CLASServer &srv = CLASServer::GetInstance();
  REQUIRE(&srv == &CLASServer::GetInstance());
}


TEST_CASE("Initialise", "[CLASServer]") {
  CLASServer &srv = CLASServer::GetInstance();
  // Check the initialisation state
  REQUIRE(srv.Status(CLASServer::StatusBits::SERVER_STARTED) == false);
  REQUIRE(srv.Status(CLASServer::StatusBits::GLOBAL_SHUTDOWN) == false);
}


