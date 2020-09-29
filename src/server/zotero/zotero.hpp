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
      
      std::string GetCitation() override
      {
        return metadata_.value("citation","");
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
      IReferenceManager::Error SendRequest(std::string requestURI,std::shared_ptr<std::unordered_map<std::string,IReference*>> &ref);

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

      std::string &GetLibraryVersion();

      void abort()
      {
        err_.exchange(IReferenceManager::SERVER_SHUTDOWN_INTERRUPT);
      }

      std::atomic<IReferenceManager::Error> &GetState()
      {
        return err_;
      }
    private:
      CURL *_curl;				///<Interface to the ssl/https api
      std::string _baseRequest;	///<Contains the basic url and group number for the zotero request

      std::string _nextLink;		///<Contains an non empty string if the json downloaded is only a part of the full json
			std::string body_;
      std::atomic<IReferenceManager::Error> err_;
      std::string libraryVersion_;

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

      using container_t = IReferenceManager::container_t;
      using ptr_t = IReferenceManager::ptr_t;
      using ptr_cont_t = IReferenceManager::ptr_cont_t;


       
      ZoteroReferenceManager(EventManager *event_manager, std::filesystem::path filename="zoteroCacheData.json");
      ~ZoteroReferenceManager();

      Error GetAllItems(ptr_cont_t &items, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
      Error GetAllCollections(ptr_cont_t &collections, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
      
      Error GetItemMetadata(ptr_t &item, std::string itemKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
      Error GetCollectionMetadata(ptr_t &collection, std::string collectionKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
     
      Error SaveToFile() override;

      /**
       * @brief Constructor for the reference manager creates the manager from
       * the given details
       *
       * @param details The details to connect the api with
       */
      IReferenceManager::Error Initialise(nlohmann::json details);

      /**
       * @brief Constructs the ReferenceManager from the specified options from
       * the given file
       *
       * @param p The path to the json file used for configuration
       *
       * @return The error if there is any return ReturnCode::OK if everything
       * went well.
       */
      IReferenceManager::Error Initialise(std::filesystem::path p);


      /**
       * @brief Returns an open connection to the zotero api to do requests with
       *
       * @return The unique pointer to an open connection
       */
      std::shared_ptr<ZoteroConnection> GetConnection();

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

      std::filesystem::path cache_path_;


      ptr_cont_t itemReferences_;
      ptr_cont_t collectionReferences_;
      std::map<std::string,int> cache_age_;

      std::shared_mutex exclusive_swap_;


      Error __tryCacheHit(ptr_cont_t &input, ptr_cont_t &ret_val, IReferenceManager::CacheOptions opts);
      
      Error __tryCacheHit(ptr_cont_t &input, ptr_t &ret_val, const std::string &value, CacheOptions opts);


      Error __performRequestsAndUpdateCache(ptr_cont_t &input, std::vector<std::string> &requestMatrix);

      void __updateCache(ptr_cont_t &input, ptr_cont_t &new_val);
      void __loadCacheFromFile();
  };
}

#endif
