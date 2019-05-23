#include "zotero/zotero.hpp"

size_t zoteroReadBuffer(void *pReceiveData, size_t pSize, size_t pNumBlocks, void *userData)
{
	Zotero *zot = (Zotero*)userData;
	zot->ReceiveBytes(reinterpret_cast<char*>(pReceiveData),pSize*pNumBlocks);
	return pSize*pNumBlocks;
}

size_t zoteroHeaderReader(char *pReceiveData, size_t pSize, size_t pNumBlocks, void *userData)
{
	//We put the Zotero pointer inside user data so cast it back to the original class
	Zotero *zot = (Zotero*)userData;

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
		DBG_INF_MSG_EXIT("Backoff request from zotero!",EXIT_FAILURE);
	}
	else if(ass.find("Retry-After: ")!=std::string::npos)
	{
		DBG_INF_MSG_EXIT("Retry after request from zotero!",EXIT_FAILURE);
	}
	//Tell libcurl how many bytes we processed.
	return pSize*pNumBlocks;
}

void Zotero::SetNextLink(std::string str)
{
	_nextLink = std::move(str);
}

Zotero::Zotero()
{
	_nextLink="";
	//Init the interface to libcurl
	_curl = curl_easy_init();

	//If we cant get a connection to libcurl exit the programm with an
	//appropriate error message
	if(!_curl)
	{
		DBG_INF_MSG_EXIT("Could not create curl interface!",EXIT_FAILURE);
	}

	//Try to load the file with the API Key from zotero and the group number
	//to perform requests to the zotero api interface
	std::ifstream ifs(ZOTERO_API_KEY_FILE_PATH,std::ios::in);
	nlohmann::json zotAccess;
	if(!ifs.is_open())
	{
		debug::print("Could not open: ",ZOTERO_API_KEY_FILE_PATH," for reading API Keys for zotero");
		throw 0;
	}

	//As the data is crucial for the programm exit the programm when there is an
	//error with reading this data
	try
	{
		ifs>>zotAccess;
	}
	catch(...)
	{
		debug::print("Faulty json read as Zotero API Key from: ",ZOTERO_API_KEY_FILE_PATH);
		throw 0;
	}
	ifs.close();


	//Calculate the base url of every zotero request which is:
	//https://api.zotero.org/groups/$(our_group_number)
	_baseRequest = ZOTERO_API_ADDR;
	_baseRequest += "/groups/";
	_baseRequest += zotAccess["groupnumber"];

	//Tell libcurl that we use a custom function to read the bytes from the socket
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, zoteroReadBuffer);

	//Set the headers used in every request to zotero, like the api version and
	//the access key to the library
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Zotero-API-Version: 3"); 
	std::string st = "Zotero-API-Key: ";
	st+=zotAccess["uniqueaccesskey"];
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


std::string Zotero::SendRequest(std::string requestURI)
{
	//Try to build the request to zotero
	//_baseRequest = https://api.zotero.org/groups/$(our_group_number)
	std::string st = _baseRequest+requestURI;


	//Create the json class to start filling
	nlohmann::json js;
	while(true)
	{
		if(debug::gGlobalShutdown)
		{
			alx::cout<<"Zotero SendRequest() read global shutdown flag, shuttding down..."<<alx::endl;
			return "{}";
		}
		debug::print("Zotero send request to url: ",st.c_str());	
		//Set the request url
		curl_easy_setopt(_curl, CURLOPT_URL, st.c_str());


		//Do the request, this request is sequential so it will only terminate
		//after the request is done. This function calls all the custom functions
		//defined in the constructor like zoteroReadBuffer and zoteroHeaderReader
		CURLcode result = curl_easy_perform(_curl);
		//If there is a fault show it in the standard error output and return an empty string
		if(result != CURLE_OK)
		{
			DBG_INF_MSG("Could not perform request to zotero api");
			_requestJSON = "";
			return std::move(_requestJSON);
		}

		//We could receive a corrupted json file
		//to handle all error catch all error and try to process them
		try
		{
			//If there is nothing in the json yet create a new json
			if(js.size()==0)
				js = nlohmann::json::parse(_requestJSON);
			else
			{
				//If there is some data in the json yet expand the json by all the
				//new elements in the newest received json
				auto j3 = nlohmann::json::parse(_requestJSON); 
				for (auto& el : j3.items()) {
					js.push_back(el.value());
				}
			}
		}
		catch(...)
		{

			std::cout<<"REQUEST JSON: "<<_requestJSON<<std::endl;
			if(_requestJSON.find("An error occurred")!=std::string::npos)
			{
				_requestJSON="";
				continue;
			}
			//TODO: Some error handling at the moment the programm just exits with an
			//error code
			DBG_INF_MSG_EXIT("Received corrupted json file!",EXIT_FAILURE);
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
		_requestJSON = "";
	}
	//Convert the json to an string and return it.
	return std::move(js.dump());
}


Zotero::~Zotero()
{
	//If there is something to clean up, clean up and reset the pointer
	if(_curl)
	{
		curl_easy_cleanup(_curl);
		_curl = nullptr;
	}
}

void Zotero::ReceiveBytes(char *pBytes, size_t pNumBytes)
{
	//Load the new data in the buffer for it
	for(size_t i = 0; i < pNumBytes; i++)
		_requestJSON += pBytes[i];
}

std::string Zotero::Request::GetSpecificItem(std::string key)
{
	std::string ret = "/items/";
	ret+=key;
	ret+="?format=json&include=data,bib,citation";
	return std::move(ret);
}

std::string Zotero::Request::GetItemsInSpecificPillar(std::string key)
{
	std::string ret = "/collections/";
	ret+=key;
	ret+="/items?format=json&include=bib,citation,data";
	return std::move(ret);
}

#ifdef COMPILE_UNITTEST

TEST(Zotero,init)
{
	EXPECT_NO_THROW(Zotero zot);
}

/**
 * This test should check if the zotero connection works by doing a simple
 * request and searching for some key which is always present in this request.
 */
TEST(Zotero,zotero_test)
{
	//Create the connection to the server
	Zotero zot;

	//Do the most basic request in zotero
	std::string request = std::move(zot.SendRequest(Zotero::Request::GetAllPillars));
	//Parse the string to a json and search for the unique key always
	//present in this request
	auto js = nlohmann::json::parse(request);
	bool exist = false;
	for(auto &el : js.items())
	{
		if(el.value()["key"].get<std::string>()=="2SWXSFCX")
			exist = true;
	}
	//The key should be present
	ASSERT_EQ(exist,true);
}

#endif
