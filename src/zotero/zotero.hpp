#pragma once
#include <curl/curl.h>
#include <fstream>
#include <streambuf>
#include "include/util/debug.hpp"
#include "json.hpp"

#define ZOTERO_API_ADDR "https://api.zotero.org"

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

	public:
		/**
		 * Creates a new ssl connection to the zotero server
		 */
		Zotero();


		/**
		 * Receives the json for a specific request.
		 * Example: SendRequest("/collections/top?format=json") returns all the
		 * top level collection form zotero in the json format in a string
		 * @param requestURI Returns the zotero json for the specific request URI.
		 * @return Returns the json for the request, returns an empty string for an
		 * invalid request.
		 *
		 */
		std::string SendRequest(std::string requestURI);

		/**
		 * Closes all open connection and cleans everything up
		 */
		~Zotero();

		friend size_t zoteroHeaderReader(char *, size_t, size_t, void *);
		friend size_t zoteroReadBuffer(void *, size_t, size_t, void *);
};
