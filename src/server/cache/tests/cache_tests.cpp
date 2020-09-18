#include <cache/cache.h>
#include <catch2/catch.hpp>
#include <memory>
#include <thread>

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

TEST_CASE("Test Cache multiple threads add","[cache]")
{
  FixedSizeCache<std::string> cache(1024*1024);
  
  std::thread t1([&cache](){
      for(int i = 0; i < 1000; i++)
      {
        cache.insert("file"+std::to_string(i), std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
      }
      });

  std::thread t2([&cache](){
      for(int i = 0; i < 1000; i++)
      {
        cache.insert("file"+std::to_string(i), std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
      }
      });
 
  std::thread t3([&cache](){
      for(int i = 0; i < 1000; i++)
      {
        cache.insert("file"+std::to_string(i), std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
      }
      });
  t1.join();
  t2.join();
  t3.join();
  REQUIRE( cache.size() == 1000 );
}

TEST_CASE("Test Cache multiple threads add diffrent","[cache]")
{
  FixedSizeCache<std::string> cache(1024*1024);
  
  std::thread t1([&cache](){
      for(int i = 0; i < 1000; i++)
      {
        cache.insert("file"+std::to_string(i), std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
      }
      });

  std::thread t2([&cache](){
      for(int i = 2000; i < 3000; i++)
      {
        cache.insert("file"+std::to_string(i), std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
      }
      });
 
  std::thread t3([&cache](){
      for(int i = 4000; i < 5000; i++)
      {
        cache.insert("file"+std::to_string(i), std::make_unique<UnmutableCacheableFile>("conaninfo.txt"));
      }
      });
  t1.join();
  t2.join();
  t3.join();
  REQUIRE( cache.size() == 3000 );
}
