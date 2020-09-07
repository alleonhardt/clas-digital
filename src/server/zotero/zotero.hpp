/**
 * @file src/zotero/zotero.hpp
 * @brief Contains the zotero interface, with which the server communicates with the zotero server
 *
 */
#ifndef CLASDIGITAL_SRC_SERVER_ZOTERO_ZOTERO_H
#define CLASDIGITAL_SRC_SERVER_ZOTERO_ZOTERO_H

#include <nlohmann/json.hpp>
#include <httplib.h>
#include <filesystem>

#include "debug/debug.hpp"
#include "reference_management/IReferenceManager.h"

namespace Zotero
{
  /**
   * @brief The possible return codes returned by the diffrent classes in the
   * zotero namespace
   */
  enum class ReturnCode
  {
    /**
     * @brief No error everything went well
     */
    OK = 0,

    /**
     * @brief No API Key found in the json file loaded or in the details given
     */
    NO_API_KEY = 1,

    /**
     * @brief No user or group id found in the configuration json
     */
    NO_GROUP_ID_OR_USER_ID = 2,


    /**
     * @brief There is an user as well as an group id in the configuration json
     * there is no way to know which one to use, therefore the error
     */
    USER_ID_AND_GROUP_ID = 4,

    /**
     * @brief Trying to load a json file that does not exist
     */
    JSON_FILE_DOES_NOT_EXIST = 8,


    /**
     * @brief Received a json that was not valid
     */
    NOT_A_VALID_JSON = 16
  };

  /**
   * @brief The basic zotero api address can be changed to make accesses to a
   * diffrent zotero api address
   */
  constexpr const char ZOTERO_API_ADDR[] = "https://api.zotero.org";

  /**
   * @brief This class represents a connection to the zotero api server and has
   * the authorization keys set already. The object is reusable therefore one
   * can make as many requests as one wants with one object. The object is not
   * thread safe DO not attempt to use the same object in diffrent threads
   * without synchronization measures.
   */
  class Connection
  {
    public:
      /**
       * @brief Constructs a connection to the given zotero server, at the given
       * api address with the given base uri and sets the api key to validate
       * requests.
       *
       * @param apiKey The api key to validate requests with
       * @param api_addr The base api address in almost all cases api.zotero.org
       * @param baseUri The base uri this is constructed by the reference
       * manager for example /groups/8123123
       */
      Connection(std::string apiKey, std::string api_addr, std::string baseUri);


      /**
       * @brief Sends a request to the given uri for example /items/all
       *
       * @param uri The ressource to request please refer to the zotero api to
       * find the fitting uri for the request
       *
       * @return The returned ressource as string
       */
      std::string SendRequest(std::string uri);

    private:
      /**
       * @brief The base uri to use for all requests for example for group abc
       * it would be /groups/abc
       */
      std::string baseURI_;

      /**
       * @brief The connection to the zotero server in order to do some
       * requests.
       */
      httplib::Client connection_;
  };


  /**
   * @brief Implements the interface of the reference manager to become a fully
   * fledged reference manager for the zotero subsystem
   */
  class ReferenceManager : public IReferenceManager
  {
    public:
      /**
       * @brief Updates the corpus from the given details.
       *
       * @param details Always required for the function to work {"name":"individual name id" or "group_name": "individual group id", "api_key": "api key"}
       */
      bool UpdateCorpus() override;

      /**
       * @brief Returns a list of references to all managed items
       *
       * @return The references to all managed items
       */
      nlohmann::json &references() override;

      /**
       * @brief Constructor for the reference manager creates the manager from
       * the given details
       *
       * @param details The details to connect the api with
       */
      ReturnCode Initialise(nlohmann::json details);

      /**
       * @brief Constructs the ReferenceManager from the specified options from
       * the given file
       *
       * @param p The path to the json file used for configuration
       *
       * @return The error if there is any return ReturnCode::OK if everything
       * went well.
       */
      ReturnCode Initialise(std::filesystem::path p);


      /**
       * @brief Returns an open connection to the zotero api to do requests with
       *
       * @return The unique pointer to an open connection
       */
      std::unique_ptr<Connection> GetConnection();

    private:
     
      /**
       * @brief The base uri to access e. g.
       * https://api.zotero.com/groups/812332 for example
       */
      std::string baseUri_;

      /**
       * @brief The api key used to validate all requests
       */
      std::string apiKey_;

      /**
       * @brief The references to all books managed by the server
       */
      nlohmann::json references_;


      /**
       * @brief The pillars that are used by the zotero programm
       */
      nlohmann::json pillars_;
  };
}

#endif
