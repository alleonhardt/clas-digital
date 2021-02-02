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
#include "filehandler/filehandler.hpp"

namespace clas_digital
{
  /**
   * @brief The basic zotero api address can be changed to make accesses to a
   * diffrent zotero api address
   */
  constexpr const char ZOTERO_API_ADDR[] = "https://api.zotero.org";
  static std::vector<std::string> split2(std::string str, std::string delimiter)
  {
    std::vector<std::string> vStr;

    size_t pos=0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
      if(pos!=0)
        vStr.push_back(str.substr(0, pos));
      str.erase(0, pos + delimiter.length());
    }
    vStr.push_back(str);

    return vStr;
  }

/**
* @param[in, out] str string to be modified
*/
static std::string returnToLower(std::string &str)
{
    std::locale loc1("de_DE.UTF8");
    std::string str2;
    for(unsigned int i=0; i<str.length(); i++)
        str2 += tolower(str[i], loc1);

    return str2;
}


  class ZoteroReference : public IReference
  {
    private: 
      std::string _path;
      nlohmann::json metadata_;

    public:
      ZoteroReference(IFileHandler *handler, nlohmann::json js) {
        metadata_ = js; 
        for(auto &vec: handler->GetUploadPoints()) {
          auto path = vec / "books" / GetKey();
          if(std::filesystem::exists(path)) 
            _path = path.string();
        }

        if(_path == "") {
          std::error_code ec;
          std::filesystem::create_directory(handler->GetUploadPoints()[0]/GetKey(),ec);
          _path = handler->GetUploadPoints()[0]/GetKey();
        }
      }

      ZoteroReference(std::string path, nlohmann::json js) {
        metadata_ = js;
        _path = path;
        if(!std::filesystem::exists(_path)) {
          std::cout<<"Zotero path does not exist "<<_path<<std::endl;
          std::exit(-1);
        }
      }
      
      std::vector<std::string> GetCollections() override
      {
        if (metadata_.count("data") == 0) 
          return {};
        return metadata_["data"].value("collections", std::vector<std::string>());
      }

      std::string GetKey() override
      {
        return metadata_.value("key","");
      }
      
      std::string GetCitation() override
      {
        return metadata_.value("citation","");
      }
      /**
       * Get whether author needs to be shown according to book creator Type.
       * @param[in] creator_type
       * @return whether to show author in catalogue
       */
      bool IsAuthorEditor(std::string creator_type) {
        if (GetMetadata("itemType", "data") == "bookSection")
          return creator_type == "author";
        return creator_type == "author" || creator_type == "editor";
      }

      /**
       * @brief return data from json.
       * @param[in] search (which metadata (f.e. title, date...)
       * @return string 
       */
      std::string GetMetadata(std::string search) {
        std::string returnSearch = metadata_.value(search, "");
        return returnSearch;
      } 

      /**
       * @brief return data from metadata.
       * @param[in] search (which metadata (f.e. title, date...)
       * @param[in] from (from which json (f.e. title -> data -> title) 
       * @return string 
       */
      std::string GetMetadata(std::string search, std::string from) {
        if (metadata_.count(from) == 0)
          return "";
        return metadata_[from].value(search, ""); 
      }

      /**
       * @brief return data from metadata
       * @param[in] search (which metadata (f.e. title, date...)
       * @param[in] from1 (from which json (f.e. title -> data -> title) 
       * @param[in] from2 (from which json (f.e. author -> data -> creators -> author)
       * @return string 
       */
      std::string GetMetadata(std::string search, std::string from1, 
          std::string from2) {
        if (metadata_.count(from1) == 0)
          return "";
        if (metadata_[from1].count(from2) == 0)
          return "";
        return metadata_[from1][from2].value(search, "");
      } 

      /**
       * @brief return data from metadata
       * @param[in] search (which metadata (f.e. title, date...)
       * @param[in] from1 (from which json (f.e. title -> data -> title) 
       * @param[in] from2 (from which json (f.e. author -> data -> creators -> author)
       * @param[in] in (in case of list: which element from list)
       * @return string 
       */
      std::string GetMetadata(std::string search, std::string from1,
          std::string from2, int in) {
        if (metadata_.count(from1) == 0)
          return "";
        if (metadata_[from1].count(from2) == 0)
          return "";
        if (metadata_[from1][from2].size() == 0)
          return "";
        return metadata_[from1][from2][in].value(search, "");
      }

      std::string GetBibliography() override {
        return metadata_.value("bib","Could not find bilbiography");
      }

      virtual bool HasContent() override {
        int count = 0;
        for(auto &it : std::filesystem::directory_iterator(GetPath())) {
          count++;
        }
        return count > 1;
      }

      std::string GetShow2() override {
        bool html = true;
        // *** Add Author *** //
        std::string result = GetAuthor();
        if (result == "") 
          result = "Unknown author";

        // *** Add title *** //
        if (GetTitle() != "" && html == true)
          result += ", <i>";
        else if (GetTitle() != "")
          result += ", ";

        //Add first [num] words of title
        std::vector<std::string> words_in_title = split2(GetTitle(), " "); 
        for (unsigned int i=0; i<10 && i<words_in_title.size(); i++)
          result += words_in_title[i] + " ";

        //Do some formatting
        result.pop_back();
        if (html == true)
          result+="</i>";
        if (words_in_title.size() > 10)
          result += "...";

        // *** Add date *** //
        if (GetDate() != -1)
          result += ", " + std::to_string(GetDate());
        return result + ".";
      }

      std::string GetAuthor() override
      {
        //Get author. Try to find "lastName"
        std::string author = GetMetadata("lastName", "data", "creators", 0);

        //If string is empty, try to find "name"
        if (author.size() == 0)
          author = GetMetadata("name", "data", "creators", 0);

        //Return result: either Name of author or empty string
        return author;
      }

      std::vector<std::map<std::string,std::string>> GetAuthorKeys() override {
        if (metadata_.count("data") == 0 || metadata_["data"].count("creators") == 0)
          return std::vector<std::map<std::string, std::string>>();

        std::vector<std::map<std::string, std::string>> v_authors;
        //Iterate and push map including "lastname", "fullname" and "key"
        for (const auto &it : metadata_["data"]["creators"]) {
          std::map<std::string, std::string> author;

          //Generate last name [0]
          std::string lastName = it.value("lastName", it.value("name", ""));
          author["lastname"] = lastName;

          //Generate  lastName, firstname [1]
          author["fullname"] = lastName;
          std::string firstname = it.value("firstName", "");
          if (firstname != "")
            author["fullname"] += ", " + firstname;

          //Generate key firstname-lastName
          std::string key = returnToLower(firstname) 
            + "-"
            + returnToLower(lastName);
          std::replace(key.begin(), key.end(), ' ', '-');
          std::replace(key.begin(), key.end(), '/', ',');
          author["key"] = key;

          author["creator_type"] = it.value("creatorType", "undefined");

          //Add to results
          v_authors.push_back(author);
        }
        return v_authors;
      }
      
      std::string GetShortTitle() override
      {
        std::string short_title = GetMetadata("shortTitle", "data");
        if (short_title == "")
          return GetTitle();
        return short_title;
      }

      std::string GetTitle() override
      {
        //Get title
        std::string title = GetMetadata("title", "data");

        //Escape html and return 
        return title;
      }

      std::string GetName() override {
        return "no name";
      }

      const nlohmann::json &json()
      {
        return metadata_;
      }

      IReference *json(nlohmann::json js)
      {
        metadata_ = std::move(js);
        return this;
      }

      int GetDate() override
      {
        //Create regex
        std::regex date3(".*(\\d{3}).*");
        std::regex date4(".*(\\d{4}).*");
        std::smatch m;

        //Get date from json
        std::string date = GetMetadata("date", "data");

        //Check whether regex find match with a 4-digit. 
        if (std::regex_search(date, m, date4))
          return std::stoi(m[1]);
        //Check whether regex find match with a 3-digit. 
        if (std::regex_search(date, m, date3))
          return std::stoi(m[1]);

        //Return -1 if regex nether matched
        return -1;
      }

      bool HasCopyright() override
      {
        return GetDate()<1922; 
      }

      std::string GetPath() override
      {
        return _path;  
      }

      IReference *Copy() override
      {
        auto ref = new ZoteroReference(_path, json());
        return ref;
      }

      virtual ~ZoteroReference() {
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
      IReferenceManager::Error SendRequest(std::string requestURI,std::shared_ptr<std::unordered_map<std::string,IReference*>> &ref, IFileHandler *_handler);

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


       
      ZoteroReferenceManager(EventManager *event_manager, IFileHandler *handler, std::filesystem::path filename="zoteroCacheData.json");

      Error GetAllItems(ptr_cont_t &items, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
      Error GetAllCollections(ptr_cont_t &collections, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
      
      Error GetItemMetadata(ptr_t &item, std::string itemKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
      Error GetCollectionMetadata(ptr_t &collection, std::string collectionKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) override;
     
      Error SaveToFile() override;
      virtual ~ZoteroReferenceManager() {}

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
      debug::CleanupDtor shutdownCallbackHandle_;

      std::filesystem::path cache_path_;


      ptr_cont_t itemReferences_;
      ptr_cont_t collectionReferences_;
      std::map<std::string,int> cache_age_;

      std::shared_mutex exclusive_swap_;
      IFileHandler *_handler;


      Error __tryCacheHit(ptr_cont_t &input, ptr_cont_t &ret_val, IReferenceManager::CacheOptions opts);
      
      Error __tryCacheHit(ptr_cont_t &input, ptr_t &ret_val, const std::string &value, CacheOptions opts);


      Error __performRequestsAndUpdateCache(ptr_cont_t &input, std::vector<std::string> &requestMatrix);

      void __updateCache(ptr_cont_t &input, ptr_cont_t &new_val);
      void __loadCacheFromFile();
  };
}

#endif
