#include "EventManager.hpp"
#include <atomic>

using namespace clas_digital;

EventManager::EventManager(CLASServer *server)
{
  server_ = server;
  id_.store(0);
}

debug::Error<EventManager::ReturnValues> EventManager::RegisterForEvent(Events event, debug::CleanupDtor &livetime, callback_t callback)
{
  unsigned long long handler;
  auto err = RegisterForEvent(event,&handler,std::move(callback));
  if(err.GetErrorCode() == RET_OK)
  {
    livetime.SetFunction([this,handler,event](){
        this->EraseEventHandler(event, &handler);
        });
  }
  return err;
}

debug::Error<EventManager::ReturnValues> EventManager::RegisterForEvent(Events event , unsigned long long *handler, callback_t callback)
{
  if(static_cast<int>(event) >= NumberOfEvents)
    return debug::Error(RET_ERROR_EVENT_DOES_NOT_EXIST);
  long long unique_id = id_.fetch_add(1);
  callbacks_[static_cast<int>(event)].insert({unique_id,std::move(callback)});

  if(handler)
    *handler = unique_id;

  return debug::Error(RET_OK);
}

CLASServer *EventManager::GetServerMainFrame()
{
  return server_;
}

debug::Error<EventManager::ReturnValues> EventManager::TriggerEvent(Events event, void *data)
{
  int iEvent = static_cast<int>(event);
  if(iEvent >= NumberOfEvents)
    return debug::Error(RET_ERROR_EVENT_DOES_NOT_EXIST);

  
  for(auto it = callbacks_[iEvent].begin(); it != callbacks_[iEvent].end();)
  {
    auto current_it = it++;
    auto err = (current_it->second)(server_,data);

    if(err.GetErrorCode() == RET_OK_DELETE_HANDLER)
      callbacks_[iEvent].erase(current_it->first);
    else if(err.GetErrorCode() == RET_ERR_DELETE_HANDLER)
    {
      err.print();
      callbacks_[iEvent].erase(current_it->first);
    }
    else if(err.GetErrorCode() == RET_ERR)
      err.print();
  }

  return debug::Error(RET_OK);
}


void EventManager::EraseEventHandler(Events event, const unsigned long long *handler)
{
  int iEvent = static_cast<int>(event);
  if(iEvent >= NumberOfEvents)
    return;
  if(handler)
    callbacks_[iEvent].erase(*handler);
}
