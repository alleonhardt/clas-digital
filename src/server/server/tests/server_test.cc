#include "server/server.hpp"
#include <catch2/catch.hpp>


TEST_CASE("Constructor", "[CLASServer]") {
  CLASServer &srv = CLASServer::GetInstance();
  REQUIRE(&srv == &CLASServer::GetInstance());
}


TEST_CASE("Initialise", "[CLASServer]") {
  CLASServer &srv = CLASServer::GetInstance();
  REQUIRE(srv.Status(CLASServer::StatusBits::SERVER_STARTED) == false);
  REQUIRE(srv.Status(CLASServer::StatusBits::GLOBAL_SHUTDOWN) == false);
}
