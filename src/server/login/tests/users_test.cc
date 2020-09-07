#include "login/user.hpp"
#include <catch2/catch.hpp>
#include <debug/debug.hpp>

TEST_CASE("Creating Usertable and inserting values","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    //
    
    REQUIRE( tbl.AddUser("alex","pw","Alexander Leonhardt",UserAccess::ADMIN).GetErrorCode() == UserTable::ReturnCodes::OK);

    REQUIRE( tbl.AddUser("alex","pwas","Alexander Leonhardtasd",UserAccess::READ).GetErrorCode() == UserTable::ReturnCodes::USER_EXISTS);


    REQUIRE( tbl.RemoveUser("alex") == false );
    REQUIRE( tbl.AddUser("alex","pwas","Alexander Leonhardtasd",UserAccess::READ) == false);

    REQUIRE( tbl.GetNumUsers() == 2 );
}


TEST_CASE("Threading user table test","[UserTable]") {
    UserTable tbl;
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::ReturnCodes::OK);
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
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::ReturnCodes::OK);
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
    REQUIRE(tbl.Load().GetErrorCode() == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    
    REQUIRE( tbl.GetNumUsers() == 1);
    nlohmann::json js = tbl.GetAsJSON();

    REQUIRE( js.dump() != "{}" );
    REQUIRE( js.size() == tbl.GetNumUsers());
    bool root_exists = false;
    for(auto &i : js) {
      if(i["email"].get<std::string>() == "root") {
        root_exists = true;
        break;
      }
    }

    REQUIRE(root_exists == true);
}

using namespace debug;

Error<int> Check()
{
  return Error<int>(0);
}

Error<int> Check2()
{
  auto err = Check();
  return Error<int>(9,"Cant initialise user table").Next(err);
}

Error<UserTable::ReturnCodes> Check3()
{
  auto err = Check2();
  return Error<UserTable::ReturnCodes>(UserTable::ReturnCodes::USER_EXISTS,"Initialise server failed").Next(err);
}


TEST_CASE("Debug StackableError test","[err_test]")
{
  debug::Error err(1, "Bad error message");
  REQUIRE(err == true);
  
  auto ch = Check3();
  REQUIRE(ch.GetErrorCode() == UserTable::ReturnCodes::USER_EXISTS);
  ch.print();
}
