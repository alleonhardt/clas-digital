#include "EventManager.hpp"

using namespace clas_digital;

debug::Error<EventManager::ReturnValues> EventManager::RegisterForEvent(Events event, callback_t callback)
{
  if(static_cast<int>(event) >= NumberOfEvents)
    return debug::Error(RET_ERROR_EVENT_DOES_NOT_EXIST);
  callbacks_[static_cast<int>(event)].push_back(std::move(callback));
  return debug::Error(RET_OK);
}

debug::Error<EventManager::ReturnValues> EventManager::TriggerEvent(Events event, CLASServer* srv, void *data)
{
  if(static_cast<int>(event) >= NumberOfEvents)
    return debug::Error(RET_ERROR_EVENT_DOES_NOT_EXIST);

  
  for(auto &it : callbacks_[static_cast<int>(event)])
  {
    auto err = it(srv,data);
    if(err.GetErrorCode() == RET_OK_DELETE_HANDLER)
      it = nullptr;
    else if(err.GetErrorCode() == RET_ERR_DELETE_HANDLER)
    {
      err.print();
      it = nullptr;
    }
    else if(err.GetErrorCode() == RET_ERR)
      err.print();
  }

  return debug::Error(RET_OK);
}


