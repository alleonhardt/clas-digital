#include "zotero/zotero.hpp"
#include "debug/debug.hpp"
#include <fstream>
#include <chrono>
#include <thread>

using namespace Zotero;



Connection::Connection(std::string apiKey, std::string api_addr, std::string baseUri) : baseURI_(baseUri),connection_(api_addr.c_str())
{
  // Sets the default headers for every Zotero request
  // Every Zotero request must include the api version and the api key
  // associated with the user that makes the request
  connection_.set_default_headers({
              { "Zotero-API-Version", "3" },
              { "Zotero-API-Key",apiKey}
  });
}

std::string Connection::SendRequest(std::string uri)
{
  // Do a request to the base uri which is probably /groups/913432/ or
  // /user/324324/ added to the real uri that is asked for
  auto resp = connection_.Get((baseURI_+uri).c_str());

  // If the status is not 200 an error occured return an empty string
  if (resp->status != 200)
  {
    debug::log(debug::LOG_LEVEL::ERRORS,"Zotero api request error received status %d\n",resp->status);
    return "";
  }

  // Check if the zotero api is overloaded, if so back of for the specified
  // amount of time
  if (resp->has_header("Backoff"))
  {
    auto seconds = resp->get_header_value("Backoff");
    debug::log(debug::LOG_LEVEL::DEBUG,"Zotero api backoff for %s seconds.\n",seconds.c_str());
    std::this_thread::sleep_for(std::chrono::seconds(std::stoi(seconds.c_str())));
  }
  
  // Retry after is send while the zotero server has maintenance therefore wait
  // the specified amount of time when encountering such an header
  if (resp->has_header("Retry-After"))
  {
    auto seconds = resp->get_header_value("Retry-After");
    debug::log(debug::LOG_LEVEL::DEBUG,"Zotero api retry after for %s seconds.\n",seconds.c_str());
    std::this_thread::sleep_for(std::chrono::seconds(std::stoi(seconds.c_str())));
  }

  if(resp->body.find("An error occurred")!=std::string::npos)
  {
    auto retry = SendRequest(uri);

    // If we have the same error again stop trying and return an empty string
    if(retry.find("An error occurred")!=std::string::npos)
    {
      debug::log(debug::LOG_LEVEL::WARNING,"Received multiple times An error occured from zotero api!\n");
      return "";
    }

    // Ok we got the new body change the body of the failed request
    *const_cast<std::string*>(&resp->body) = std::move(retry);
  }

  // If there is an link in the response the whole response is not parsed yet,
  // continue with the next response
  if (resp->has_header("Link"))
  {
    auto link = resp->get_header_value("Link");

    // The format is Link: something; <https://somelink> rel="next"; something
    // Therefore try to find the next link there is
    auto pos = link.find("rel=\"next\"");

    // If there is no next link the response is finished
		if(pos!=std::string::npos)
		{
      // Try to determine the positions of the next link location
			auto pos1 = link.rfind("<",pos);
			auto pos2 = link.rfind(">",pos);
			//Extract the next address to do a request to and set it in the zotero
			//class
			std::string st2 = link.substr(pos1+1,pos2-(pos1+1));

      // Return the whole response back to the requester
			return resp->body + SendRequest(st2);
		}
  }

  // Return the response
  return resp->body;
}

ReturnCode Zotero::ReferenceManager::Initialise(std::filesystem::path p)
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


ReturnCode Zotero::ReferenceManager::Initialise(nlohmann::json details)
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

  // Everything went well!
  return ReturnCode::OK;
}

std::unique_ptr<Connection> ReferenceManager::GetConnection()
{
  // Create a new connection to the zotero api server with the required
  // parameters
  std::unique_ptr<Connection> ptr(new Connection(apiKey_,ZOTERO_API_ADDR,baseUri_));
  return ptr;
}


bool ReferenceManager::UpdateCorpus()
{
  //TODO: implement the update corpus function
  return true;
}

nlohmann::json &ReferenceManager::references()
{
  return references_;
}
