#include "login/user.hpp"
#include <catch2/catch.hpp>
#include <debug/debug.hpp>

using namespace clas_digital;

TEST_CASE("Creating Usertable and inserting values","[UserTable]") {
    UserTable tbl;
    debug::LogClass::gLogLevel = debug::LogLevel::DBG_TODO;
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::RET_OK);
    //Let the destructor run to write changes on disk!
    //
    
    nlohmann::json cr;
    cr["email"] = "alex";
    cr["password"] = "pw";

    REQUIRE( tbl.AddUser(cr).GetErrorCode() == UserTable::RET_OK);

    REQUIRE( tbl.AddUser(cr).GetErrorCode() == UserTable::RET_USER_EXISTS);


    REQUIRE( tbl.RemoveUserByKey("alex") == false );
    REQUIRE( tbl.AddUser(cr) == false);

    REQUIRE( tbl.GetNumUsers() == 1 );
}


TEST_CASE("Threading user table test","[UserTable]") {
    debug::LogClass::gLogLevel = debug::LogLevel::DBG_ALWAYS;
    UserTable tbl;
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::RET_OK);
    //Let the destructor run to write changes on disk!
    
    std::thread t1([&tbl](){
        nlohmann::json js;
          for(int i = 0; i < 500; i++)
          {
            js["email"] = std::to_string(i);
            js["password"] = "leo";
            js["access"] = User::ACC_ADMIN;

            tbl.AddUser(js);
          }
        });
    
    std::thread t2([&tbl](){
        nlohmann::json js;
        for(int i = 1000; i < 1500; i++)
        {
          js["email"] = std::to_string(i);
          js["password"] = "leo";
          js["access"] = User::ACC_ADMIN;

          tbl.AddUser(js);
          }
        });

    std::thread t3([&tbl](){
        nlohmann::json js;
        for(int i = 2000; i < 2500; i++)
        {
          js["email"] = std::to_string(i);
          js["password"] = "leo";
          js["access"] = User::ACC_ADMIN;

          tbl.AddUser(js);
          }
        });

    t1.join();
    t2.join();
    t3.join();

    REQUIRE( tbl.GetNumUsers() == 1500 ); 
}

TEST_CASE("Check root user","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::RET_OK);
    //Let the destructor run to write changes on disk!
    
    REQUIRE(tbl.GetNumUsers() == 0);
    
    nlohmann::json cr;
    cr["email"] = "alex";
    cr["password"] = "pw";
    REQUIRE(tbl.AddUser(cr).GetErrorCode() == UserTable::RET_OK);

    REQUIRE(tbl.GetNumUsers() == 1);


    std::string cookie = tbl.LogIn(cr);
    REQUIRE(cookie != "");

    //Must be at least 32 bytes long otherwise its a very weak cookie
    REQUIRE(cookie.length() >= 32);

    auto usr = tbl.GetUserFromCookie(cookie);
    REQUIRE(usr.get() != nullptr);
    REQUIRE(usr->GetPrimaryKey() == "alex");
}

TEST_CASE("Check UserTable to json","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::RET_OK);
    //Let the destructor run to write changes on disk!
    
    nlohmann::json cr;
    cr["email"] = "alex";
    cr["password"] = "pw";
    
    REQUIRE( tbl.AddUser(cr).GetErrorCode() == UserTable::RET_OK );
    
    REQUIRE( tbl.GetNumUsers() == 1);

    nlohmann::json js = tbl.GetAsJSON();

    REQUIRE( js.dump() != "{}" );
    REQUIRE( js.size() == tbl.GetNumUsers());
    bool root_exists = false;
    for(auto &i : js) {
      if(i["email"].get<std::string>() == "alex") {
        root_exists = true;
        break;
      }
    }

    REQUIRE(root_exists == true);
}

