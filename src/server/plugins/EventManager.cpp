#include "EventManager.hpp"

using namespace cl_events;

bool EventManager::RegisterForEvent(Events event, callback_t callback)
{
  if(static_cast<int>(event) >= NumberOfEvents)
    return false;
  callbacks_[static_cast<int>(event)].push_back(std::move(callback));
  return true;
}

debug::Error<EventReturn> EventManager::TriggerEvent(Events event, CLASServer* srv, void *data)
{
  if(static_cast<int>(event) >= NumberOfEvents)
    return debug::Error(EventReturn::ERROR_EVENT_DOES_NOT_EXIST);

  for(auto &it : callbacks_[static_cast<int>(event)])
  {
    auto err = it(srv,data);
    if(err.GetErrorCode() == EventReturn::ERR_ABORT_FOLLOWING)
      return err;
    else if(err.GetErrorCode() == EventReturn::OK_ABORT_FOLLOWING)
    {
      err.SetErrorCode(EventReturn::OK);
      return err;
    }
    else if(err.GetErrorCode() == EventReturn::ERROR)
    {
      err.print();
    }
  }

  return debug::Error(EventReturn::OK);
}


