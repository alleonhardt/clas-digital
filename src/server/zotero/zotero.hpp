/**
 * @file src/zotero/zotero.hpp
 * @brief Contains the zotero interface, with which the server communicates with the zotero server
 *
 */
#ifndef CLASDIGITAL_SRC_SERVER_ZOTERO_ZOTERO_H
#define CLASDIGITAL_SRC_SERVER_ZOTERO_ZOTERO_H

#include <nlohmann/json.hpp>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <curl/curl.h>
#include <vector>
#include <filesystem>
#include <shared_mutex>

#include "debug/debug.hpp"
#include "reference_management/IReferenceManager.h"

namespace clas_digital
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


  class ZoteroReference : public IReference
  {
    public:
      std::string GetKey() override
      {
        return metadata_.value("key","");
      }

      std::string GetAuthor() override
      {
        return "none";
      }

      std::string GetShortTitle() override
      {
        return "none";
      }

      std::string GetTitle() override
      {
        return "none";
      }

      int GetDate() override
      {
        return 10;
      }

      bool HasCopyright() override
      {
        return false;
      }

      IReference *Copy() override
      {
        auto ref = new ZoteroReference();
        ref->json(metadata_);
        return ref;
      }
  };


  size_t zoteroReadBuffer(void *pReceiveData, size_t pSize, size_t pNumBlocks, void *userData);


  size_t zoteroHeaderReader(char *pReceiveData, size_t pSize, size_t pNumBlocks, void *userData);

  /**
   * @brief The zotero class connects the server to the zotero api and requests metadata from the server to keep the metadata up to date
   *
   */
  class ZoteroConnection
  {
    public:
      /**
       * Receives the json for a specific request.
       * Example: _sendRequest("/collections/top?format=json") returns all the
       * top level collection form zotero in the json format in a string
       * @param requestURI Returns the zotero json for the specific request URI.
       * @return Returns the json for the request, returns an empty string for an
       * invalid request.
       *
       */
      ReturnCode SendRequest(std::string requestURI,std::shared_ptr<std::unordered_map<std::string,IReference*>> &ref);

      /**
       * Closes all open connection and cleans everything up
       */
      ~ZoteroConnection();

      friend size_t zoteroHeaderReader(char *, size_t, size_t, void *); ///< Needed as curl callback on header receiving
      friend size_t zoteroReadBuffer(void *, size_t, size_t, void *);  ///< Needed as curl callback on data receiving
   
      /**
       * Creates a new ssl connection to the zotero server
       */
      ZoteroConnection(std::string apiKey, std::string api_addr, std::string baseUri);

    private:
      CURL *_curl;				///<Interface to the ssl/https api
      std::string _baseRequest;	///<Contains the basic url and group number for the zotero request

      std::string _nextLink;		///<Contains an non empty string if the json downloaded is only a part of the full json
			std::string body_;

      /**
       * Writes the buffer inside of the class variable _requestJSON
       * @param pBytes The buffer which contains the last received json
       * @param pNumBytes The number of bytes inside the buffer
       */

      void ReceiveBytes(char *pBytes, size_t pNumBytes);

      /**
       * Sets the class variable _nextLink to the next url which is needed to download
       * all json chunks from zotero
       * @param str The next url to receive the json from
       */
      void SetNextLink(std::string str);
  };



  /**
   * @brief Implements the interface of the reference manager to become a fully
   * fledged reference manager for the zotero subsystem
   */
  class ZoteroReferenceManager : public IReferenceManager
  {
    public:
      using container_t = std::unordered_map<std::string,IReference*>;
      using ptr_t = std::shared_ptr<IReference>;
      using ptr_cont_t = std::shared_ptr<container_t>;

      using func = IReferenceManager::func;
       
      ZoteroReferenceManager();

      Error GetAllItems(func on_item, CacheOptions opts=CacheOptions::CACHE_USE_CACHED);
      Error GetAllCollections(func on_collections, CacheOptions opts=CacheOptions::CACHE_USE_CACHED);
      Error GetAllItemsFromCollection(func on_item, std::string collectionKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED);
      
      Error GetItemMetadata(func on_item, std::string item, CacheOptions opts=CacheOptions::CACHE_USE_CACHED);
      Error GetCollectionMetadata(func on_item, std::string collection, CacheOptions opts=CacheOptions::CACHE_USE_CACHED);

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
      std::unique_ptr<ZoteroConnection> GetConnection();

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
      std::vector<std::string> trackedCollections_;
      std::string citationStyle_;


      ptr_cont_t itemReferences_;
      ptr_cont_t collectionReferences_;
      std::shared_mutex exclusive_swap_;


      Error __applyForEach(const ZoteroReferenceManager::ptr_cont_t &t, IReferenceManager::func &fnc, IReferenceManager::CacheOptions opts);

      Error __updateContainerAndApplyForEach(ZoteroReferenceManager::ptr_cont_t &destination, ZoteroReferenceManager::ptr_cont_t &source, IReferenceManager::func &fnc);

      Error __updateContainerAndApply(ZoteroReferenceManager::ptr_cont_t &destination, ZoteroReferenceManager::ptr_cont_t &source, IReferenceManager::func &fnc);

      Error __applyFuncOnSingleElement(ZoteroReferenceManager::ptr_cont_t &destination, const std::string &key, IReferenceManager::func &fnc, IReferenceManager::CacheOptions opts);
  };
}

#endif
