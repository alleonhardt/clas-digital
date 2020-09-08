#include "plugins/EventManager.hpp"
#include <catch2/catch.hpp>
#include <debug/debug.hpp>

TEST_CASE("EventManager register event","[EventManager]")
{
  cl_events::EventManager evt;
  int x = 0;
  cl_events::EventManager::callback_t cl = [&x](CLASServer* srv,void* data){
    ++x;
    REQUIRE(srv == nullptr);
    REQUIRE(data == nullptr);
    return debug::Error(cl_events::EventReturn::OK);
  };

  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, cl) == true);

  REQUIRE(evt.TriggerEvent(cl_events::Events::ON_UPDATE_REFERENCE, nullptr, nullptr).GetErrorCode() == cl_events::EventReturn::OK);

  REQUIRE( x == 1 );


  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, std::move(cl)) == true);
  
  REQUIRE(evt.TriggerEvent(cl_events::Events::ON_UPDATE_REFERENCE, nullptr, nullptr).GetErrorCode() == cl_events::EventReturn::OK);
  
  //Two times increase
  REQUIRE( x == 3 );
}

TEST_CASE("EventManager Abort Following without error","[EventManager]")
{
  cl_events::EventManager evt;
  int x = 0;
  cl_events::EventManager::callback_t cl = [&x](CLASServer* srv,void* data){
    ++x;
    REQUIRE(srv == nullptr);
    REQUIRE(data == nullptr);
    return debug::Error(cl_events::EventReturn::OK_ABORT_FOLLOWING);
  };

  //Register 3 function callbacks only one should be called
  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, cl) == true);
  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, cl) == true);
  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, cl) == true);

  REQUIRE(evt.TriggerEvent(cl_events::Events::ON_UPDATE_REFERENCE, nullptr, nullptr).GetErrorCode() == cl_events::EventReturn::OK);

  REQUIRE( x == 1 );
}

TEST_CASE("EventManager Abort Following with error","[EventManager]")
{
  cl_events::EventManager evt;
  int x = 0;
  cl_events::EventManager::callback_t cl = [&x](CLASServer* srv,void* data){
    ++x;
    REQUIRE(srv == nullptr);
    REQUIRE(data == nullptr);
    return debug::Error(cl_events::EventReturn::ERR_ABORT_FOLLOWING);
  };

  //Register 3 function callbacks only one should be called
  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, cl) == true);
  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, cl) == true);
  REQUIRE(evt.RegisterForEvent(cl_events::Events::ON_UPDATE_REFERENCE, cl) == true);

  REQUIRE(evt.TriggerEvent(cl_events::Events::ON_UPDATE_REFERENCE, nullptr, nullptr) == true);

  REQUIRE( x == 1 );
}


TEST_CASE("EventManager Register/Trigger Unkown event","[EventManager]")
{
  cl_events::EventManager evt;
  int x = 0;
  cl_events::EventManager::callback_t cl = [&x](CLASServer* srv,void* data){
    ++x;
    REQUIRE(srv == nullptr);
    REQUIRE(data == nullptr);
    return debug::Error(cl_events::EventReturn::ERR_ABORT_FOLLOWING);
  };

  //Register 3 function callbacks only one should be called
  REQUIRE(evt.RegisterForEvent((cl_events::Events)10, cl) == false);
  REQUIRE(evt.RegisterForEvent((cl_events::Events)-1, cl) == false);
  REQUIRE(evt.RegisterForEvent((cl_events::Events)5, cl) == false);

  REQUIRE(evt.TriggerEvent((cl_events::Events)10, nullptr, nullptr).GetErrorCode() == cl_events::EventReturn::ERROR_EVENT_DOES_NOT_EXIST);
  REQUIRE(evt.TriggerEvent((cl_events::Events)-1, nullptr, nullptr).GetErrorCode() == cl_events::EventReturn::ERROR_EVENT_DOES_NOT_EXIST);
  REQUIRE(evt.TriggerEvent((cl_events::Events)5, nullptr, nullptr).GetErrorCode() == cl_events::EventReturn::ERROR_EVENT_DOES_NOT_EXIST);

  REQUIRE( x == 0 );
}


TEST_CASE("Event Manager TriggerEvent without register, no segfault","[EventManager]")
{
  cl_events::EventManager evt;
  REQUIRE(evt.TriggerEvent(cl_events::Events::ON_UPDATE_REFERENCE, nullptr, nullptr).GetErrorCode() == cl_events::EventReturn::OK);
}
