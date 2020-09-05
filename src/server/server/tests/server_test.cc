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

TEST_CASE("Login","[CLASServer]") {
  CLASServer &srv = CLASServer::GetInstance();

  std::thread t1([&srv](){
      while(true) {
        httplib::Client cl("localhost",9786);

        auto resp = cl.Post("/api/v2/server/login", {}, "email=alex&password=none", "application/x-www-form-urlencoded");
        if(resp.error()) continue;

        REQUIRE( resp->status == 403);

        //Check the default root user used in every user database
        resp = cl.Post("/api/v2/server/login", {}, "email=root&password=password", "application/x-www-form-urlencoded");
        REQUIRE( resp->status == 200 );
        srv.Stop();
        break;
      }
      });

  srv.Start("localhost",9786);
  t1.join();
}
