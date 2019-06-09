/**
 * @file src/zotero/zotero.hpp
 * @brief Contains the zotero interface, with which the server communicates with the zotero server
 *
 */
#pragma once
#include <curl/curl.h>
#include <fstream>
#include <streambuf>
#include "src/util/debug.hpp"
#include "src/books/json.hpp"

constexpr const char ZOTERO_API_ADDR[] = "https://api.zotero.org"; ///<The zotero api server address where to send all requests to
constexpr const char ZOTERO_API_KEY_FILE_PATH[] = "bin/zoteroKey.json";	///<The file path at which the access key and the group number can be found

/**
 * @brief The zotero class connects the server to the zotero api and requests metadata from the server to keep the metadata up to date
 *
 */
class Zotero
{
	private:
		CURL *_curl;				///<Interface to the ssl/https api
		std::string _requestJSON;	///<Contains the JSON String of the current request
		std::string _baseRequest;	///<Contains the basic url and group number for the zotero request

		std::string _nextLink;		///<Contains an non empty string if the json downloaded is only a part of the full json

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

		/**
		 * Creates a new ssl connection to the zotero server
		 */
		Zotero();

		/**
		 * Receives the json for a specific request.
		 * Example: _sendRequest("/collections/top?format=json") returns all the
		 * top level collection form zotero in the json format in a string
		 * @param requestURI Returns the zotero json for the specific request URI.
		 * @return Returns the json for the request, returns an empty string for an
		 * invalid request.
		 *
		 */
		std::string _sendRequest(std::string requestURI);
	
	public:
		/**
		 * @brief Defines the most Basic requests to the zotero API
		 * @code
		 * Zotero::SendRequest(Zotero::Request::GetAllItems);
		 * //Or this
		 * Zotero::SendRequest(Zotero::Request::GetSpecificItem("X2DEFG"));
		 * @endcode
		 */
		struct Request
		{
			static constexpr const char GetAllItems[] = "/items?format=json&include=data,bib,citation&limit=100"; ///< The zotero request to get all items in the zotero library from zotero
			static constexpr const char GetAllPillars[] = "/collections/top?format=json"; ///< The zotero request to get all collections from the zotero api
			static std::string GetSpecificItem(std::string key); ///< The zotero request to get a specific item from the zotero api
			static std::string GetItemsInSpecificPillar(std::string key); ///< The zotero request to get all items from a specific collection out of zotero
		};

		/**
		 * @brief This is just a wrapper for the _sendRequest function 
		 * @param requestURI The zotero items uri which is about to be received
		 * @return The json for the request or an empty string on error
		 */
		static std::string SendRequest(std::string requestURI);

		/**
		 * Closes all open connection and cleans everything up
		 */
		~Zotero();

		static const nlohmann::json &GetPillars();
		friend size_t zoteroHeaderReader(char *, size_t, size_t, void *); ///< Needed as curl callback on header receiving
		friend size_t zoteroReadBuffer(void *, size_t, size_t, void *);  ///< Needed as curl callback on data receiving
};
