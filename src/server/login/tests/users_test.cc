#include "login/user.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Creating Usertable and inserting values","[UserTable]") {
  std::error_code ec;
  std::filesystem::remove("users.db",ec);

  {
    UserTable tbl;
    REQUIRE(tbl.Load("users.db") == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    //
    
    REQUIRE( tbl.AddUser("alex","pw","Alexander Leonhardt",UserAccess::ADMIN) == UserTable::ReturnCodes::OK);

    REQUIRE( tbl.AddUser("alex","pwas","Alexander Leonhardtasd",UserAccess::READ) == UserTable::ReturnCodes::USER_EXISTS);


    REQUIRE( tbl.RemoveUser("alex") == UserTable::ReturnCodes::OK);
  }

  std::filesystem::remove("users.db",ec);
  REQUIRE(ec.value() == 0); 
}
