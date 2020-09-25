#include "EventManager.hpp"
#include <atomic>

using namespace clas_digital;

EventManager::EventManager(CLASServer *server)
{
  server_ = server;
  id_ = 0;
}

debug::Error<EventManager::ReturnValues> EventManager::RegisterForEvent(Events event , unsigned long long *handler, callback_t callback)
{
  std::unique_lock lck(shared_mutex_);
  if(static_cast<int>(event) >= NumberOfEvents)
    return debug::Error(RET_ERROR_EVENT_DOES_NOT_EXIST);
  callbacks_[static_cast<int>(event)].insert({++id_,std::move(callback)});
  if(handler)
    *handler = id_;

  return debug::Error(RET_OK);
}

debug::Error<EventManager::ReturnValues> EventManager::TriggerEvent(Events event, void *data)
{
  std::shared_lock lck(shared_mutex_);
  int iEvent = static_cast<int>(event);
  if(iEvent >= NumberOfEvents)
    return debug::Error(RET_ERROR_EVENT_DOES_NOT_EXIST);

  
  for(auto it = callbacks_[iEvent].begin(); it != callbacks_[iEvent].end();)
  {
    auto err = (it->second)(server_,data);
    if(err.GetErrorCode() == RET_OK_DELETE_HANDLER)
      it = callbacks_[iEvent].erase(it);
    else if(err.GetErrorCode() == RET_ERR_DELETE_HANDLER)
    {
      err.print();
      it = callbacks_[iEvent].erase(it);
    }
    else if(err.GetErrorCode() == RET_ERR)
      err.print();
    else
      ++it;
  }

  return debug::Error(RET_OK);
}


void EventManager::EraseEventHandler(Events event, unsigned long long *handler)
{
  std::unique_lock lck(shared_mutex_);
  
  int iEvent = static_cast<int>(event);
  if(iEvent >= NumberOfEvents)
    return;
  if(handler)
    callbacks_[iEvent].erase(*handler);
}
