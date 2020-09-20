#include "zotero/zotero.hpp"
#include "debug/debug.hpp"
#include <fstream>
#include <chrono>
#include <thread>
#include "filehandler/util.h"
#include "reference_management/IReferenceManager.h"
#include "server/server.hpp"

using namespace clas_digital;

size_t clas_digital::zoteroReadBuffer(void *pReceiveData, size_t pSize, size_t pNumBlocks, void *userData)
{
	ZoteroConnection *zot = (ZoteroConnection*)userData;
	zot->ReceiveBytes(reinterpret_cast<char*>(pReceiveData),pSize*pNumBlocks);
	return pSize*pNumBlocks;
}


size_t clas_digital::zoteroHeaderReader(char *pReceiveData, size_t pSize, size_t pNumBlocks, void *userData)
{
	//We put the Zotero pointer inside user data so cast it back to the original class
	ZoteroConnection *zot = (ZoteroConnection*)userData;

	//Load the string with one line from the http header,
	//The function zoteroHeaderReader will get called for each header in the http request
	std::string ass;
	ass.assign(pReceiveData,pSize*pNumBlocks);

  if(ass.find("HTTP/1.1 404")!=std::string::npos)
  {
    zot->err_ = IReferenceManager::Error::KEY_DOES_NOT_EXIST;
	  return pSize*pNumBlocks;
  }
  else if(ass.find("HTTP/1.1 403")!=std::string::npos)
  {
    zot->err_ = IReferenceManager::Error::API_KEY_NOT_VALID_OR_NO_ACCESS;
	  return pSize*pNumBlocks;
  }

	//Look if there is a link to follow in order to download the whole json
	if(ass.find("Link: ")!=std::string::npos)
	{
		//Ok so there is a link to the next chunk of the json so extract the link
		//Link format:
		//		Link:	something, <https://new_address&start=x> rel="next", something
		auto pos = ass.find("rel=\"next\"");
		if(pos!=std::string::npos)
		{
			auto pos1 = ass.rfind("<",pos);
			auto pos2 = ass.rfind(">",pos);
			//Extract the next address to do a request to and set it in the zotero
			//class
			std::string st2 = ass.substr(pos1+1,pos2-(pos1+1));
			zot->SetNextLink(std::move(st2));
		}
	}

	//If there is either an Backoff or an Retry-After header
	//zotero wants us to stop doing request for a few seconds
	//TODO: Handle the headers instead of exiting the programm on finding
	//those headers
	if(ass.find("Backoff: ")!=std::string::npos)
	{
	}
	else if(ass.find("Retry-After: ")!=std::string::npos)
	{
	}

  const std::string last_modified_header = "Last-Modified-Version: ";
  auto position = ass.find(last_modified_header);
  if(position != std::string::npos)
  {
    auto start = position+last_modified_header.length(); 
    std::string subs = ass.substr(start, ass.find("\r\n",start)-start);
    std::cout<<"Extracted version substr: "<<subs<<std::endl;
    zot->libraryVersion_ = std::move(subs);
  }
	//Tell libcurl how many bytes we processed.
	return pSize*pNumBlocks;
}

std::string &ZoteroConnection::GetLibraryVersion()
{
  return libraryVersion_;
}

void ZoteroConnection::SetNextLink(std::string str)
{
	_nextLink = std::move(str);

}

ZoteroConnection::ZoteroConnection(std::string apiKey, std::string api_addr, std::string baseUri)
{
  err_ = IReferenceManager::Error::OK;
	_nextLink="";
	//Init the interface to libcurl
	_curl = curl_easy_init();

	//If we cant get a connection to libcurl exit the programm with an
	//appropriate error message
	if(!_curl)
	{
    throw 0;
	}


	//Calculate the base url of every zotero request which is:
	//https://api.zotero.org/groups/$(our_group_number)
	_baseRequest = api_addr;
  _baseRequest += baseUri;

	//Tell libcurl that we use a custom function to read the bytes from the socket
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, zoteroReadBuffer);

	//Set the headers used in every request to zotero, like the api version and
	//the access key to the library
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Zotero-API-Version: 3"); 
	std::string st = "Zotero-API-Key: ";
	st+=apiKey;
	headers = curl_slist_append(headers, st.c_str());
	//Transmit the custom headers to libcurl
	curl_easy_setopt(_curl,CURLOPT_HTTPHEADER,headers);

	//Set a custom Data pointer to give to our custom write function
	curl_easy_setopt(_curl,CURLOPT_WRITEDATA,this);

	//Set a custom callback function for reading the headers of the response
	//from zotero used for processing, Link, Backoff and Retry-After headers
	curl_easy_setopt(_curl,CURLOPT_HEADERFUNCTION,zoteroHeaderReader);

	//Set a the custom data pointer given to the header function to this class
	curl_easy_setopt(_curl,CURLOPT_HEADERDATA,this);
}


IReferenceManager::Error ZoteroConnection::SendRequest(std::string requestURI, std::shared_ptr<std::unordered_map<std::string,IReference*>> &ref)
{
	//Try to build the request to zotero
	//_baseRequest = https://api.zotero.org/groups/$(our_group_number)
	std::string st = _baseRequest+requestURI;
  body_.reserve(512*1024);
  body_ = "";



  //Create the json class to start filling
	while(true)
	{
		std::cout<<"Zotero request to url: "<<st<<std::endl;

		//Set the request url
		curl_easy_setopt(_curl, CURLOPT_URL, st.c_str());
		curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 30L);


		//Do the request, this request is sequential so it will only terminate
		//after the request is done. This function calls all the custom functions
		//defined in the constructor like zoteroReadBuffer and zoteroHeaderReader
		CURLcode result = curl_easy_perform(_curl);
		//If there is a fault show it in the standard error output and return an empty string
		if(result != CURLE_OK)
		{
			std::cout<<"[Could not perform request to zotero api!]\n";
			return IReferenceManager::Error::USER_ID_AND_GROUP_ID;
		}
    
    if(err_)
      return err_;
		//We could receive a corrupted json file
		//to handle all error catch all error and try to process them
		try
		{
			//If there is nothing in the json yet create a new json
	    nlohmann::json js = nlohmann::json::parse(body_);

			if(js.is_array())
      {
        for(auto &it : js)
        {
          IReference* ptr((new ZoteroReference)->json(it));
          ref->insert({ptr->GetKey(),ptr});
        }
      }
			else
      {
          IReference* ptr((new ZoteroReference)->json(js));
          ref->insert({ptr->GetKey(),ptr});
      }
		}
		catch(...)
		{
			if(body_.find("An error occurred")!=std::string::npos)
			{
				body_="";
				continue;
			}
			//TODO: Some error handling at the moment the programm just exits with an
			//error code
			return IReferenceManager::Error::NOT_A_VALID_JSON;
		}

		//If the custom header function set the link to the next chunk of the json
		//follow the link by looping over the new url
		if(_nextLink!="")
		{
			st = std::move(_nextLink);
		}
		else
		{
			//No link to follow? Close the programm.
			break;
		}

		//Clear the buffer so we dont process data multiple times
		body_ = "";
	}

	//Convert the json to an string and return it.
	return IReferenceManager::Error::OK;
}


ZoteroConnection::~ZoteroConnection()
{
	//If there is something to clean up, clean up and reset the pointer
	if(_curl)
	{
		curl_easy_cleanup(_curl);
		_curl = nullptr;
	}
}

void ZoteroConnection::ReceiveBytes(char *pBytes, size_t pNumBytes)
{
	body_.append(pBytes,pNumBytes);
}



void custom_deleter(ZoteroReferenceManager::container_t* ptr)
{
  for(auto &it : *ptr)
  {
    if(it.second)
      delete it.second;
  }
  delete ptr;
}

void ZoteroReferenceManager::__loadCacheFromFile()
{
  std::ifstream ifs(cache_path_, std::ios::in);
  try
  {
    nlohmann::json save;
    if(ifs.is_open())
    {
      ifs>>save;
      ifs.close();
    }

    auto loadFieldContainer = [&save](const char *field, IReferenceManager::ptr_cont_t &cont, std::string &libVersion)
    {
      if(save.count(field))
      {
        cont = IReferenceManager::ptr_cont_t(new container_t,custom_deleter);
        for(auto &it : save[field]["data"])
        {
          IReference *ref = new ZoteroReference();
          ref->json(it);
          cont->insert({ref->GetKey(),ref});
        }
        libVersion = save["items"]["version"];
        if(cont->size() == 0)
          cont.reset();
      }
    };

    loadFieldContainer("items",itemReferences_,libraryVersionItems_);
    loadFieldContainer("collections",collectionReferences_,libraryVersionReferences_);

  }
  catch(...)
  {
  }
}

ZoteroReferenceManager::ZoteroReferenceManager(std::filesystem::path path)
{ 
  cache_path_ = std::move(path);  
}
      
IReferenceManager::Error ZoteroReferenceManager::SaveToFile()
{
  nlohmann::json save;
  {
    std::shared_lock lck(exclusive_swap_);

    bool save_file = false;
    auto save_to_json = [&save,&save_file](const char *signature,IReferenceManager::ptr_cont_t &cont,std::string &libVersion)
    {
      save[signature] = nlohmann::json();
      save[signature]["data"] = nlohmann::json::array();
      save[signature]["version"] = libVersion;
      if(cont && cont->size() > 0)
      {
        save_file = true;
        for(auto &it : *cont)
          save[signature]["data"].push_back(it.second->json());
      }
    };
    
    save_to_json("items",itemReferences_,libraryVersionItems_);
    save_to_json("collections", collectionReferences_, libraryVersionReferences_);

    if(!save_file)
      return Error::UNKNOWN;
  }
  if(!clas_digital::atomic_write_file(cache_path_,save))
    return Error::CACHE_FILE_PATH_NOT_VALID;

  return Error::OK;
}

ZoteroReferenceManager::~ZoteroReferenceManager()
{
  SaveToFile();
}

IReferenceManager::Error ZoteroReferenceManager::Initialise(std::filesystem::path p)
{
  // Try to load the file from the specified path
  nlohmann::json js;
  std::ifstream ifs(p,std::ios::in);

  // If the file does not exist return the corresponding error
  if(ifs.fail())
    return Error::JSON_FILE_DOES_NOT_EXIST;

  try
  {
    // Try to load and parse the json from disk
    ifs>>js;
  }
  catch(...)
  {
    // Something went wrong, probably the JSON was not valid
    return Error::NOT_A_VALID_JSON;
  }

  // Close the file input and intialise with the given settings
  ifs.close();
  return Initialise(js);
}


IReferenceManager::Error ZoteroReferenceManager::Initialise(nlohmann::json details)
{
  // Check if there is an API Key, this is REQUIRED for an connection to zotero
  if (details.count("api_key") == 0)
    return Error::NO_API_KEY;
  else
    apiKey_ = details["api_key"];

  // Check if there is an group id that the implementation will use to do
  // requests to
  if (details.count("group_id") == 0)
  {
    // If there is neither an group id nor an user id return an error
    if (details.count("user_id") == 0)
      return IReferenceManager::Error::NO_GROUP_OR_USER_ID;

    //TODO: Access the users database
    baseUri_ = "/users/";
    baseUri_ += details["user_id"];
  }
  else
  {
    // If there is an user id as well there is no way to know what to use
    // therefore return an error
    if (details.count("user_id") != 0)
      return Error::USER_ID_AND_GROUP_ID;
    
    // Construct the basic uri from the group id and the basi format of zotero
    // for requests to an group 
    baseUri_ = "/groups/";
    baseUri_ += details["group_id"];
  }

  if(details.count("collections") > 0)
  {
    for(auto &it : details["collections"])
    {
      trackedCollections_.push_back(it.get<std::string>());
    }
  }

  citationStyle_ = details.value("citation_style","kritische-ausgabe");
  cache_path_ = details.value("cache_file",cache_path_.string());
  __loadCacheFromFile();

  // Everything went well!
  return Error::OK;
}

std::unique_ptr<ZoteroConnection> ZoteroReferenceManager::GetConnection()
{
  // Create a new connection to the zotero api server with the required
  // parameters
  return std::unique_ptr<ZoteroConnection>(new ZoteroConnection(apiKey_,ZOTERO_API_ADDR,baseUri_));
}




IReferenceManager::Error ZoteroReferenceManager::__tryCacheHit(ZoteroReferenceManager::ptr_cont_t &input, ZoteroReferenceManager::ptr_cont_t &ret_val, IReferenceManager::CacheOptions opts)
{
  std::shared_lock lck(exclusive_swap_);

  if(!input || opts == CacheOptions::CACHE_FORCE_FETCH)
    return Error::CACHE_MISS;

  ret_val = input; 
  return Error::OK;
}


IReferenceManager::Error ZoteroReferenceManager::__tryCacheHit(ZoteroReferenceManager::ptr_cont_t &input, ZoteroReferenceManager::ptr_t &ret_val, const std::string &value, CacheOptions opts)
{
  std::shared_lock lck(exclusive_swap_);
  if(!input || opts == CacheOptions::CACHE_FORCE_FETCH)
    return Error::CACHE_MISS;

  auto val = input->find(value);
  if(val == input->end())
    return Error::CACHE_MISS;

  ret_val = std::shared_ptr<IReference>(val->second->Copy());
  return Error::OK;
}


void ZoteroReferenceManager::__updateCache(ZoteroReferenceManager::ptr_cont_t &input, ZoteroReferenceManager::ptr_cont_t &new_val)
{
  
  if(event_manager_)
  {
    auto func = [this,new_val]()
    {
      for(auto &it : *new_val)
      {
        event_manager_->TriggerEvent(EventManager::Events::ON_UPDATE_REFERENCE, &CLASServer::GetInstance(), it.second);
      }
    };
    
    //Start a thread to do the event dispatching when encoutering a lot of new
    //items
    if(new_val->size() > 100)
      std::thread(func).detach();
    else
      func();
  }

  {
    std::unique_lock lck(exclusive_swap_);
    if(input)
    {
      for(auto &it : *new_val)
      {
        auto fnd = input->find(it.first);
        if(fnd!=input->end())
        {
          std::cout<<"Ptr second: "<<fnd->second<<std::endl;
          delete fnd->second;
          fnd->second = it.second;
        }
        else
          (*input)[it.first] = it.second;
      }
    }
    else
      input = std::move(new_val);
  }
}

IReferenceManager::Error ZoteroReferenceManager::__performRequestsAndUpdateCache(ZoteroReferenceManager::ptr_cont_t &input, ZoteroReferenceManager::ptr_cont_t &output, std::vector<std::string> &requestMatrix,std::string &libraryVersion)
{
  ptr_cont_t request(new container_t(),custom_deleter);
  auto connection = GetConnection();
 
  for(const auto &it : requestMatrix)
  {
    auto res = connection->SendRequest(it,request);
    if(res != Error::OK)
      return res;
  }


  libraryVersion = std::move(connection->GetLibraryVersion());
  __updateCache(input,request);
  output = input;

  return Error::OK;
}


IReferenceManager::Error ZoteroReferenceManager::GetAllItems(ZoteroReferenceManager::ptr_cont_t &items, IReferenceManager::CacheOptions opts)
{
  auto err = __tryCacheHit(itemReferences_,items,opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;
 
  std::vector<std::string> cmdMatrix;
  if(trackedCollections_.size() == 0)
    cmdMatrix.push_back("/items?format=json&include=data,bib,citation&style="+citationStyle_+"&limit=100");
  else
    std::for_each(trackedCollections_.begin(),trackedCollections_.end(),[&cmdMatrix,this](std::string &coll){cmdMatrix.push_back("/collections/"+coll+"/items?format=json&include=data,bib,citation&style="+this->citationStyle_+"&limit=100");});

  return __performRequestsAndUpdateCache(itemReferences_,items,cmdMatrix,libraryVersionItems_);
 }

IReferenceManager::Error ZoteroReferenceManager::GetAllCollections(IReferenceManager::ptr_cont_t &collections, IReferenceManager::CacheOptions opts)
{ 
  auto err = __tryCacheHit(collectionReferences_,collections,opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;
  
  std::vector<std::string> cmdMatrix;
  if(trackedCollections_.size() == 0)
    cmdMatrix.push_back("/collections?format=json&include=data&limit=100");
  else
    std::for_each(trackedCollections_.begin(),trackedCollections_.end(),[&cmdMatrix](std::string &coll){cmdMatrix.push_back("/collections/"+coll+"?format=json&include=data");});

  return __performRequestsAndUpdateCache(collectionReferences_, collections, cmdMatrix,libraryVersionReferences_);
}


IReferenceManager::Error ZoteroReferenceManager::GetItemMetadata(ZoteroReferenceManager::ptr_t &item, std::string key, IReferenceManager::CacheOptions opts)
{
  auto err = __tryCacheHit(itemReferences_, item, key, opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;

  auto connection = GetConnection();
  ptr_cont_t request(new container_t());
  auto res = connection->SendRequest("/items/"+key+"?format=json&include=data,bib,citation&style="+citationStyle_,request);

  if(res == Error::OK)
  {
    item = std::shared_ptr<IReference>(request->begin()->second);
    return Error::OK;
  }
  return res;
}


IReferenceManager::Error ZoteroReferenceManager::GetCollectionMetadata(IReferenceManager::ptr_t &collection, std::string collectionKey, IReferenceManager::CacheOptions opts)
{
  auto err = __tryCacheHit(collectionReferences_, collection, collectionKey, opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;

  auto connection = GetConnection();
  ptr_cont_t request(new container_t());
  auto res = connection->SendRequest("/collections/"+collectionKey+"?format=json&include=data",request);

  if(res == Error::OK)
  {
    collection = std::shared_ptr<IReference>(request->begin()->second);
  }
  return res;
}
