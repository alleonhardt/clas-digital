#include "login/user.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Creating Database","[UserTable]") {
  std::error_code ec;
  std::filesystem::remove("users.db",ec);

  UserTable tbl;
  REQUIRE(tbl.Load("users.db") == true);

  std::filesystem::remove("users.db",ec);
  REQUIRE(ec.value() == 0); 
}
