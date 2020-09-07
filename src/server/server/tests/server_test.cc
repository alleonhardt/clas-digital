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
      while(!srv.IsRunning())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
      httplib::Client cl("localhost",9786);
      cl.set_connection_timeout(2);

      auto resp = cl.Post("/api/v2/server/login", {}, "email=alex&password=none", "application/x-www-form-urlencoded");

      REQUIRE( resp->status == 403);

      //Check the default root user used in every user database
      resp = cl.Post("/api/v2/server/login", {}, "email=root&password=password", "application/x-www-form-urlencoded");
      REQUIRE( resp->status == 200 );

      //check if a cookie was send with the response;
      REQUIRE(resp->get_header_value("Set-Cookie").length() > 32);
      std::string ck = resp->get_header_value("Set-Cookie");
      auto pos = ck.find(";");

      httplib::Headers headers = {
      { "Cookie", resp->get_header_value("Set-Cookie").substr(0,pos+1) }
      };
      cl.set_default_headers(headers);

      resp = cl.Get("/api/v2/server/userlist");
      REQUIRE( resp->status == 200 );
      REQUIRE( resp->body.length() > 0 );

      srv.Stop();
      });

  while(srv.Start("localhost",9786) == CLASServer::ReturnCodes::ERR_PORT_BINDING) {
  }
  t1.join();
}



TEST_CASE("Update User List","[CLASServer]") {
  CLASServer &srv = CLASServer::GetInstance();

  std::thread t1([&srv](){
      while(!srv.IsRunning())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
      httplib::Client cl("localhost",9786);
      cl.set_connection_timeout(2);

      //Check the default root user used in every user database
      auto resp = cl.Post("/api/v2/server/login", {}, "email=root&password=password", "application/x-www-form-urlencoded");
      REQUIRE( resp->status == 200 );

      //check if a cookie was send with the response;
      REQUIRE(resp->get_header_value("Set-Cookie").length() > 32);
      std::string ck = resp->get_header_value("Set-Cookie");
      auto pos = ck.find(";");

      httplib::Headers headers = {
      { "Cookie", resp->get_header_value("Set-Cookie").substr(0,pos+1) }
      };
      cl.set_default_headers(headers);

      {
        nlohmann::json change_table;
        nlohmann::json entry;
        entry["action"] = "CREATE";
        entry["email"] = "alex";
        entry["password"] = "password";
        entry["access"] = (int)UserAccess::ADMIN;
        change_table.push_back(entry);

        resp = cl.Post("/api/v2/server/userlist",change_table.dump(),"application/json");
        REQUIRE( resp->status == 200 );

        resp = cl.Post("/api/v2/server/login", "email=alex&password=password", "application/x-www-form-urlencoded");

        REQUIRE( resp->status == 200 );
        REQUIRE(resp->get_header_value("Set-Cookie").length() > 32);
      }

      {
        nlohmann::json change_table;
        nlohmann::json entry;
        entry["action"] = "DELETE";
        entry["email"] = "root";
        change_table.push_back(entry);

        resp = cl.Post("/api/v2/server/userlist",change_table.dump(),"application/json");
        REQUIRE(resp->status == 200);

        resp = cl.Post("/api/v2/server/login", {}, "email=root&password=password", "application/x-www-form-urlencoded");
        REQUIRE(resp->status == 403); 
      }
      srv.Stop();
      return;
      });

  while(srv.Start("localhost",9786) == CLASServer::ReturnCodes::ERR_PORT_BINDING) {
  }
  t1.join();
}
