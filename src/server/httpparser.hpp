#pragma once
/**
 * @file src/server/httpparser.hpp
 *
 * Classes for parsing requests and constructing reponses in the http format
 *
 * This header defines the interface to the classes http_request and http_reponse
 */
#include <string>
#include <vector>
#include <string_view>
#include <iostream>
#include <map>

/**
 * Parses a given string in the xhttp html format and puts the results in the given map.
 * @param[out] map The map to put the results from the parsing into
 * @param[in] query The string to parse into the map.
 */
void parse_query(std::map<std::string,std::string> &map,std::string_view const &query);

/**
 * Transform two hexadecimal digits into the character they represent eg tochar('2','0') = 0x20.
 * @param[in] hi The high byte of the character
 * @param[in] lo The low byte of the character
 * @return The calculated byte
 */
unsigned char tochar(char hi, char lo);


/**
 * @brief Parses and stores informations about an HTTP request
 */
class http_request
{
	public:
		/**
		 * Creates an http request given a buffer returned by an read operation and the size of the buffer.
		 * The class automatically parses query parameters path, url, method body size and pointer to body. While doing so it does not copy anything therefore it is up to the programmer to ensure the buffer given to the http_request constructor remains unchanged while the http_request is in use. If there is an error while parsing the http file the IsHealthy() function will return false
		 * @param[in] asyncReadBuf The Read buffer containing the informations about the http request.
		 * @param[in] bytes The length of the buffer.
		 */
		http_request(const char *asyncReadBuf,size_t bytes);

		/**
		 * Prints all the information gathered in the constructor about the http request, mainly used for debug purposes
		 */
		void print_request();

		/**
		 * Returns the headers parsed from the http request.
		 * @param key The name of the http parameter to search for
		 * @return The header fitting to the key if there is one
		 */
		const std::string_view &GetHeaders(std::string key);

		/**
		 * Returns the query parameter parsed from the url given the key to search for.
		 * @param key The variable name in the query parameter
		 * @return The query parameter fitting to a key.
		 */
		const std::string_view GetQueryParams(std::string key);
		
		/**
		 * Returns the method used in the Request, 'POST' or 'GET' etc.
		 * @return The method used for the message
		 */
		const std::string_view &GetMethod();
		
		/**
		 * Returns the URL parsed from the request
		 * @return The full url
		 */
		const std::string_view &GetURL();
		
		/**
		 * Returns the mean path from the url /search?query=hallo
		 * will return /search.
		 * @return The path extracted from the url.
		 */
		const std::string_view &GetPath();
		
		/**
		 * Returns the whole unparsed query string. Request to /search?query=hallo
		 * will return query=hallo
		 * @return The whole query string
		 */
		const std::string_view &GetQuery();
		
		/**
		 * Returns if the http message parsed is healthy and does not miss something
		 * @return Returns if the http message is healthy
		 */
		bool IsHealthy();

		/**
		 * Returns the body of the http message, this pointer is not owned by the class, therefore it should not be modified at all.
		 * @return Returns the immutable body buffer
		 */
		const void *GetBody();

		/**
		 * Returns the body size stored inside the class
		 * @return The body size of the class.
		 */
		unsigned long GetBodySize();
	private:
		std::string_view _method;	///<A view of the method the http request is
		std::string_view _url;		///<The url the request refers to
		std::string_view _path;		///<The absolute URI the request refers to
		std::string_view _query;	///<The full unparsed query part of the URL
		std::map<std::string,std::string> _queryMap;	///<The parsed query part of the URL
		std::map<std::string_view,std::string_view> _headers;	///<The parsed headers of the HTTP request
		bool _healthy;	///<Checks if the message is well built and the class didnt encounter errors while parsing
		const void *_body;	///<A pointer to the body part of the HTTP message
		unsigned long _bodySize;	///<The body size of the http message
};
