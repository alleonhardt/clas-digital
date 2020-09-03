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

TEST_CASE("StartServer", "[CLASServer]") {
  CLASServer &srv = CLASServer::GetInstance();
  
  // Starting the server and checking on tear down that the global shutdown flag
  // was set
  std::thread t1([&srv](){
      srv.Start("localhost",1234);
      REQUIRE(srv.Status(CLASServer::StatusBits::GLOBAL_SHUTDOWN) == true);
      });


  // Server should be started right now and global shutdown should be set to
  // false
  REQUIRE(srv.Status(CLASServer::StatusBits::GLOBAL_SHUTDOWN) == false);
  
  // Set the global shutdown flag stop the server and wait up for the thread
  // join
  srv.Status(CLASServer::StatusBits::GLOBAL_SHUTDOWN,true);
  srv.Stop();
  t1.join();
}


