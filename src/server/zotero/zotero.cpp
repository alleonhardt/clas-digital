#include "zotero/zotero.hpp"
#include "debug/debug.hpp"
#include <fstream>
#include <chrono>
#include <thread>

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
	//Tell libcurl how many bytes we processed.
	return pSize*pNumBlocks;
}

void ZoteroConnection::SetNextLink(std::string str)
{
	_nextLink = std::move(str);

}

ZoteroConnection::ZoteroConnection(std::string apiKey, std::string api_addr, std::string baseUri)
{
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


ReturnCode ZoteroConnection::SendRequest(std::string requestURI, std::shared_ptr<std::unordered_map<std::string,IReference*>> &ref)
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
			return ReturnCode::USER_ID_AND_GROUP_ID;
		}

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
			return ReturnCode::NO_GROUP_ID_OR_USER_ID;
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
	return ReturnCode::OK;
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

ZoteroReferenceManager::ZoteroReferenceManager() : itemReferences_(new ZoteroReferenceManager::container_t()), collectionReferences_(new ZoteroReferenceManager::container_t())
{ 
}

ReturnCode ZoteroReferenceManager::Initialise(std::filesystem::path p)
{
  // Try to load the file from the specified path
  nlohmann::json js;
  std::ifstream ifs(p,std::ios::in);

  // If the file does not exist return the corresponding error
  if(ifs.fail())
    return ReturnCode::JSON_FILE_DOES_NOT_EXIST;

  try
  {
    // Try to load and parse the json from disk
    ifs>>js;
  }
  catch(...)
  {
    // Something went wrong, probably the JSON was not valid
    return ReturnCode::NOT_A_VALID_JSON;
  }

  // Close the file input and intialise with the given settings
  ifs.close();
  return Initialise(js);
}


ReturnCode ZoteroReferenceManager::Initialise(nlohmann::json details)
{
  // Check if there is an API Key, this is REQUIRED for an connection to zotero
  if (details.count("api_key") == 0)
    return ReturnCode::NO_API_KEY;
  else
    apiKey_ = details["api_key"];

  // Check if there is an group id that the implementation will use to do
  // requests to
  if (details.count("group_id") == 0)
  {
    // If there is neither an group id nor an user id return an error
    if (details.count("user_id") == 0)
      return ReturnCode::NO_GROUP_ID_OR_USER_ID;

    //TODO: Access the users database
    baseUri_ = "/users/";
    baseUri_ += details["user_id"];
  }
  else
  {
    // If there is an user id as well there is no way to know what to use
    // therefore return an error
    if (details.count("user_id") != 0)
      return ReturnCode::USER_ID_AND_GROUP_ID;
    
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

  // Everything went well!
  return ReturnCode::OK;
}

std::unique_ptr<ZoteroConnection> ZoteroReferenceManager::GetConnection()
{
  // Create a new connection to the zotero api server with the required
  // parameters
  return std::unique_ptr<ZoteroConnection>(new ZoteroConnection(apiKey_,ZOTERO_API_ADDR,baseUri_));
}




IReferenceManager::Error ZoteroReferenceManager::__applyForEach(const ZoteroReferenceManager::ptr_cont_t &t, IReferenceManager::func &fnc, IReferenceManager::CacheOptions opts)
{
  std::shared_lock lck(exclusive_swap_);
  if(!t || opts == CacheOptions::CACHE_FORCE_FETCH)
    return Error::CACHE_MISS;

  for(auto &it : *t)
    fnc(it.second);
  
  return Error::OK;
}

IReferenceManager::Error ZoteroReferenceManager::__updateContainerAndApplyForEach(ZoteroReferenceManager::ptr_cont_t &destination, ZoteroReferenceManager::ptr_cont_t &source, IReferenceManager::func &fnc)
{
  {
    std::unique_lock lck(exclusive_swap_);
    destination = std::move(source);
  }

  std::shared_lock lck(exclusive_swap_);
  for(auto &it : *destination)
    fnc(it.second);

  return Error::OK;
}

IReferenceManager::Error ZoteroReferenceManager::__updateContainerAndApply(ZoteroReferenceManager::ptr_cont_t &destination, ZoteroReferenceManager::ptr_cont_t &source, IReferenceManager::func &fnc)
{
  
  {
    IReference *ref = nullptr;
    std::unique_lock lck(exclusive_swap_);

    for(auto &it : *source)
    {
      std::swap((*destination)[it.first], it.second);
      if(it.second)
        delete it.second;
    }
  }

  std::shared_lock lck(exclusive_swap_);
  for(auto &it : *destination)
    fnc(it.second);

  return Error::OK;
}

IReferenceManager::Error ZoteroReferenceManager::__applyFuncOnSingleElement(ZoteroReferenceManager::ptr_cont_t &destination, const std::string &key, IReferenceManager::func &fnc, IReferenceManager::CacheOptions opts)
{
  std::shared_lock lck(exclusive_swap_);
  if(!destination || opts == CacheOptions::CACHE_FORCE_FETCH)
    return Error::CACHE_MISS;
  
  try
  {
    fnc(destination->at(key));
    return Error::OK;
  }
  catch(...)
  {
    return Error::CACHE_MISS;
  }
}




IReferenceManager::Error ZoteroReferenceManager::GetAllItems(IReferenceManager::func on_item, IReferenceManager::CacheOptions opts)
{
  auto err = __applyForEach(itemReferences_,on_item,opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;
 
  ptr_cont_t request(new container_t(),custom_deleter);
  auto connection = GetConnection();
  
  std::vector<std::string> cmdMatrix;
  if(trackedCollections_.size() == 0)
    cmdMatrix.push_back("/items?format=json&include=data,bib,citation&style="+citationStyle_+"&limit=100");
  else
    std::for_each(trackedCollections_.begin(),trackedCollections_.end(),[&cmdMatrix,this](std::string &coll){cmdMatrix.push_back("/collections/"+coll+"/items?format=json&include=data,bib,citation&style="+this->citationStyle_+"&limit=100");});

  for(const auto &it : cmdMatrix)
  {
    auto res = connection->SendRequest(it,request);
    if(res != ReturnCode::OK)
      return Error::UNKNOWN;
  }

  return __updateContainerAndApplyForEach(itemReferences_,request,on_item);
}

IReferenceManager::Error ZoteroReferenceManager::GetAllCollections(IReferenceManager::func on_item, IReferenceManager::CacheOptions opts)
{ 
  auto err = __applyForEach(collectionReferences_,on_item,opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;
  
  ptr_cont_t request(new container_t(),custom_deleter);
  auto connection = GetConnection();

  std::vector<std::string> cmdMatrix;
  if(trackedCollections_.size() == 0)
    cmdMatrix.push_back("/collections?format=json&include=data&limit=100");
  else
    std::for_each(trackedCollections_.begin(),trackedCollections_.end(),[&cmdMatrix](std::string &coll){cmdMatrix.push_back("/collections/"+coll+"?format=json&include=data");});

  for(const auto &it : cmdMatrix)
  {
    auto res = connection->SendRequest(it,request);
    if(res != ReturnCode::OK)
      return Error::UNKNOWN;
  }

  return __updateContainerAndApplyForEach(collectionReferences_,request,on_item);
}


IReferenceManager::Error ZoteroReferenceManager::GetItemMetadata(IReferenceManager::func on_item, std::string item, IReferenceManager::CacheOptions opts)
{
  auto err = __applyFuncOnSingleElement(itemReferences_,item,on_item,opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;

  auto connection = GetConnection();
  ptr_cont_t request(new container_t());
  auto res = connection->SendRequest("/items/"+item+"?format=json&include=data,bib,citation&style="+citationStyle_,request);

  if(res == ReturnCode::OK)
    return __updateContainerAndApply(itemReferences_,request, on_item);
  
  return Error::UNKNOWN;
}


IReferenceManager::Error ZoteroReferenceManager::GetCollectionMetadata(IReferenceManager::func on_item, std::string collection, IReferenceManager::CacheOptions opts)
{
  auto err = __applyFuncOnSingleElement(collectionReferences_,collection, on_item,opts);
  if(err == Error::OK || opts == CacheOptions::CACHE_FAIL_ON_CACHE_MISS) return err;

  auto connection = GetConnection();
  ptr_cont_t request(new container_t());
  auto res = connection->SendRequest("/collections/"+collection+"?format=json&include=data",request);

  if(res == ReturnCode::OK)
    return __updateContainerAndApply(collectionReferences_,request, on_item);
  return Error::UNKNOWN;
}

IReferenceManager::Error ZoteroReferenceManager::GetAllItemsFromCollection(IReferenceManager::func on_item, std::string collectionKey, IReferenceManager::CacheOptions opts)
{
  ptr_cont_t request(new container_t());
  auto connection = GetConnection();

  auto res = connection->SendRequest("/collections/"+collectionKey+"/items?format=json&include=data,bib,citation&style="+citationStyle_+"&limit=100",request);
  
  if(res == ReturnCode::OK)
    return __updateContainerAndApply(itemReferences_,request, on_item);

  return Error::UNKNOWN;
}

