#include "plugins/EventManager.hpp"
#include <catch2/catch.hpp>
#include <debug/debug.hpp>

using namespace clas_digital;

TEST_CASE("EventManager register event","[EventManager]")
{
  EventManager evt;
  int x = 0;
  EventManager::callback_t cl = [&x](CLASServer* srv,void* data){
    ++x;
    REQUIRE(srv == nullptr);
    REQUIRE(data == nullptr);
    return debug::Error(EventManager::RET_OK);
  };

  REQUIRE(evt.RegisterForEvent(EventManager::ON_UPDATE_REFERENCE, cl) == false);

  REQUIRE(evt.TriggerEvent(EventManager::ON_UPDATE_REFERENCE, nullptr, nullptr).GetErrorCode() == EventManager::RET_OK);

  REQUIRE( x == 1 );


  REQUIRE(evt.RegisterForEvent(EventManager::ON_UPDATE_REFERENCE, std::move(cl)) == false);
  
  REQUIRE(evt.TriggerEvent(EventManager::ON_UPDATE_REFERENCE, nullptr, nullptr).GetErrorCode() == EventManager::RET_OK);
  
  //Two times increase
  REQUIRE( x == 3 );
}



TEST_CASE("EventManager Register/Trigger Unkown event","[EventManager]")
{
  EventManager evt;
  int x = 0;
  EventManager::callback_t cl = [&x](CLASServer* srv,void* data){
    ++x;
    REQUIRE(srv == nullptr);
    REQUIRE(data == nullptr);
    return debug::Error(EventManager::RET_ERR);
  };

  //Register 3 function callbacks only one should be called
  REQUIRE(evt.RegisterForEvent((EventManager::Events)10, cl) == true);
  REQUIRE(evt.RegisterForEvent((EventManager::Events)-1, cl) == true);
  REQUIRE(evt.RegisterForEvent((EventManager::Events)5, cl) == true);

  REQUIRE(evt.TriggerEvent((EventManager::Events)10, nullptr, nullptr).GetErrorCode() == EventManager::RET_ERROR_EVENT_DOES_NOT_EXIST);
  REQUIRE(evt.TriggerEvent((EventManager::Events)-1, nullptr, nullptr).GetErrorCode() == EventManager::RET_ERROR_EVENT_DOES_NOT_EXIST);
  REQUIRE(evt.TriggerEvent((EventManager::Events)12, nullptr, nullptr).GetErrorCode() == EventManager::RET_ERROR_EVENT_DOES_NOT_EXIST);

  REQUIRE( x == 0 );
}


TEST_CASE("Event Manager TriggerEvent without register, no segfault","[EventManager]")
{
  EventManager evt;
  REQUIRE(evt.TriggerEvent(EventManager::ON_UPDATE_REFERENCE, nullptr, nullptr).GetErrorCode() == EventManager::RET_OK);
}
