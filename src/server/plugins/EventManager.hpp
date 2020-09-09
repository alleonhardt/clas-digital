#ifndef CLASDIGITAL_SRC_SERVER_PLUGINS_EVENTMANAGER_H
#define CLASDIGITAL_SRC_SERVER_PLUGINS_EVENTMANAGER_H
#include <functional>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "debug/debug.hpp"


namespace clas_digital
{
  class CLASServer;
  class EventManager
  {
    public:
      enum ReturnValues
      {
        RET_OK = 0,
        RET_ERR,
        RET_ERROR_EVENT_DOES_NOT_EXIST,
        RET_ERR_DELETE_HANDLER,
        RET_OK_DELETE_HANDLER
      };

      enum Events
      {
        ON_UPDATE_REFERENCE = 0,
        ON_AFTER_INITIALISE,
        ON_SERVER_START,
        ON_SERVER_STOP
      };
      
      constexpr static unsigned long NumberOfEvents = 4;

      using callback_t = std::function<debug::Error<ReturnValues>(CLASServer *, void *)>;

      debug::Error<ReturnValues> RegisterForEvent(Events event, callback_t callback);
      debug::Error<ReturnValues> TriggerEvent(Events event, CLASServer* srv, void *data);

    private:
      std::vector<callback_t> callbacks_[NumberOfEvents];
  };
}

#endif
