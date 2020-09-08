#ifndef CLASDIGITAL_SRC_SERVER_PLUGINS_EVENTMANAGER_H
#define CLASDIGITAL_SRC_SERVER_PLUGINS_EVENTMANAGER_H
#include <functional>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "debug/debug.hpp"

class CLASServer;

namespace cl_events
{
  enum class Events
  {
    ON_UPDATE_REFERENCE = 0,
    ON_CONFIG_LOAD,
    BEFORE_SERVER_INITIALISE,
    AFTER_SERVER_INITIALISE,

  };

  constexpr unsigned long NumberOfEvents = 4;

  enum class EventReturn
  {
    OK = 0,
    OK_ABORT_FOLLOWING = 1,
    ERR_ABORT_FOLLOWING = 2,
    ERROR_EVENT_FAILED,
    ERROR_EVENT_DOES_NOT_EXIST
  };


  class EventManager
  {
    public:
      using callback_t = std::function<debug::Error<EventReturn>(CLASServer *, void *)>;

      bool RegisterForEvent(Events event, callback_t callback);
      debug::Error<EventReturn> TriggerEvent(Events event, CLASServer* srv, void *data);

    private:
      std::vector<callback_t> callbacks_[NumberOfEvents];
  };
}

#endif
