#include "login/user.hpp"
#include <catch2/catch.hpp>
#include <debug/debug.hpp>

TEST_CASE("Creating Usertable and inserting values","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load() == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    //
    
    REQUIRE( tbl.AddUser("alex","pw","Alexander Leonhardt",UserAccess::ADMIN) == UserTable::ReturnCodes::OK);

    REQUIRE( tbl.AddUser("alex","pwas","Alexander Leonhardtasd",UserAccess::READ) == UserTable::ReturnCodes::USER_EXISTS);


    REQUIRE( tbl.RemoveUser("alex") == UserTable::ReturnCodes::OK);
    REQUIRE( tbl.AddUser("alex","pwas","Alexander Leonhardtasd",UserAccess::READ) == UserTable::ReturnCodes::OK);

    REQUIRE( tbl.GetNumUsers() == 2 );
}


TEST_CASE("Threading user table test","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load() == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    
    std::thread t1([&tbl](){
        for(int i = 0; i < 500; i++)
          tbl.AddUser(std::to_string(i), "alex", "leo", UserAccess::ADMIN);
        });
    
    std::thread t2([&tbl](){
        for(int i = 2000; i < 2500; i++)
          tbl.AddUser(std::to_string(i), "alex", "leo", UserAccess::ADMIN);
        });

    std::thread t3([&tbl](){
        for(int i = 4000; i < 4500; i++)
          tbl.AddUser(std::to_string(i), "alex", "leo", UserAccess::ADMIN);
        });

    t1.join();
    t2.join();
    t3.join();

    REQUIRE( tbl.GetNumUsers() == 1501 ); 
}

TEST_CASE("Check root user","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load() == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    
    REQUIRE( tbl.GetNumUsers() == 1);
    std::string cookie = tbl.LogIn("root", "password");
    REQUIRE(cookie != "");

    //Must be at least 32 bytes long otherwise its a very weak cookie
    REQUIRE(cookie.length() >= 32);

    auto usr = tbl.GetUserFromCookie(cookie);
    REQUIRE(usr != nullptr);
    REQUIRE(usr->Email() == "root");
    REQUIRE(usr->Access() == UserAccess::ADMIN); 
}

TEST_CASE("Check UserTable to json","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load() == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    
    REQUIRE( tbl.GetNumUsers() == 1);
    nlohmann::json js = tbl.GetAsJSON();

    REQUIRE( js.dump() != "{}" );
    REQUIRE( js.size() == tbl.GetNumUsers());
    bool root_exists = false;
    for(auto &i : js) {
      std::cout<<i.dump()<<std::endl;
      if(i["email"].get<std::string>() == "root") {
        root_exists = true;
        break;
      }
    }

    REQUIRE(root_exists == true);
}
