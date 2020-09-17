#include <cache/cache.h>
#include <catch2/catch.hpp>

using namespace clas_digital;
TEST_CASE("Test the constructor","[cache]")
{
  FixedSizeCache<std::string> cache(1024*1024);
  cache.insert("file",std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
  cache.insert("file2",std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
  cache.insert("file3",std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
  cache.load("file",[](void *data,long long sz){REQUIRE(data != nullptr);},[](int){REQUIRE( false == true);});

  REQUIRE(cache.size() == 3);
  REQUIRE(cache.free_space() != cache.total_space());
}


TEST_CASE("Test Unregister","[cache]")
{
  FixedSizeCache<std::string> cache(1024*1024);
  cache.insert("file",std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
  cache.load("file",[](void *data,long long sz){REQUIRE(data != nullptr);},[](int){REQUIRE( false == true);});

  cache.erase("file");
  REQUIRE( cache.size() == 0 );
  cache.load("file",[](void*,long long){},[](int err){REQUIRE(err == 2);});
}
