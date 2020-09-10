#ifndef CLASDIGITAL_SRC_SERVER_REFERENCE_MANAGEMENT_IREFERENCE_MANAGER_H
#define CLASDIGITAL_SRC_SERVER_REFERENCE_MANAGEMENT_IREFERENCE_MANAGER_H

#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace clas_digital
{

  class IReference
  {
    public:
      const nlohmann::json &json()
      {
        return metadata_;
      }

      IReference *json(nlohmann::json js)
      {
        metadata_ = std::move(js);
        return this;
      }

      virtual IReference *Copy() = 0;
      virtual std::string GetAuthor() = 0; 
      virtual std::string GetShortTitle() = 0;
      virtual std::string GetTitle() = 0;
      virtual int GetDate() = 0;
      virtual bool HasCopyright() = 0;
      virtual std::string GetKey() = 0;

    protected:
        nlohmann::json metadata_;
  };

  /**
   * @brief Manages the reference information, for the moment only manages the
   * data from zotero but could be extended in a later version
   */
  class IReferenceManager
  {
    public:
      using func = std::function<void(IReference*)>;

      enum CacheOptions
      {
        CACHE_USE_CACHED,
        CACHE_FORCE_FETCH,
        CACHE_FAIL_ON_CACHE_MISS
      };

      enum Error
      {
        OK,
        CACHE_MISS,
        MULTIPLE_RESULTS,
        NO_JSON_RETURNED,
        UNKNOWN
      };

    
      virtual Error GetAllItems(func on_item, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;
      virtual Error GetAllCollections(func on_collections, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;
      virtual Error GetAllItemsFromCollection(func on_item, std::string collectionKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;

      virtual Error GetItemMetadata(func on_item, std::string item, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;
      virtual Error GetCollectionMetadata(func on_item, std::string collection, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;
  };

}



#endif
