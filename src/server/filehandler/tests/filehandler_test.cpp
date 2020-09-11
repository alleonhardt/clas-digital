#include "filehandler/filehandler.hpp"
#include <catch2/catch.hpp>
using namespace clas_digital;

TEST_CASE("Atomic write file","[atomic_write]")
{
  std::string val = "Hallo";
  atomic_write_file("myfile.txt",val);
  {
    std::ifstream ifs("myfile.txt",std::ios::in);
    REQUIRE(ifs.is_open() == true);
    std::string check;
    ifs>>check;
    ifs.close();
    REQUIRE(check == "Hallo");
  }
  
  val="Welt";
  atomic_write_file("myfile.txt",val);
  {
    std::ifstream ifs("myfile.txt",std::ios::in);
    REQUIRE(ifs.is_open() == true);
    std::string check;
    ifs>>check;
    ifs.close();
    REQUIRE(check == "Welt");
  }
}
