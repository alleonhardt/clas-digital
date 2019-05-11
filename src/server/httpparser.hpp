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
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>


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

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

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
		const std::string_view &GetHeaders(const std::string key);

		/**
		 * Returns the query parameter parsed from the url given the key to search for.
		 * @param key The variable name in the query parameter
		 * @return The query parameter fitting to a key.
		 */
		const std::string_view GetQueryParams(const std::string key);
		
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


/**
 * @brief This class constructs a http reponse and sends it so the client
 */
class http_response
{
	public:
		/**
		 * The Error codes thrown by various http_response functions
		 */
		enum class Errors
		{
			StatusAlreadyDefined, ///<The status was already defined when the user tried to set it
			StatusNotSet ///<The status was not set before the user tried to set the headers
		};

		/**
		 * The status codes used to answer to the client
		 */
		struct StatusCodes
		{
			constexpr static char Continue[] = "HTTP/1.1 100 Continue\r\n"; ///<Continue HTTP Header
			constexpr static char Ok[] = "HTTP/1.1 200 OK\r\n"; ///<Ok HTTP Header
			constexpr static char NotFound[] = "HTTP/1.1 404 Not found\r\n"; ///<Not found HTTP header
			constexpr static char Unauthorized[] = "HTTP/1.1 401 Unauthorized\r\n"; ///<Unauthorized http header
		};

		/**
		 * Creates the http response.
		 */
		http_response();
		
		/**
		 * Sets the status code for the response
		 * @param statusCode The statusCode for the response
		 * @return A reference to the class itself, used for function chaining
		 */
		http_response &status(const std::string hdr);

		/**
		 * Sets a new header for the pending response.
		 * @param[in] key The header field to write
		 * @param[in] value The value field which will get set after the header field
		 * @return A reference to the class itself, this enables function chaining eg response.header("Content-Length","10").header("hdr2","someVal");
		 */
		http_response &header(const std::string key, const std::string value);
		http_response &body(const std::string &bdy);
		void SendWithEOM(ssl_socket &sock);
		void handle_write_done(std::string *str,const boost::system::error_code &err, std::size_t bytes_transfered);

	private:
		std::string _statusLine; ///<The status line returned
		std::map<std::string,std::string> _headers;	///<The headers for the response
		std::string _body; 		///<The response later given to async_write
};
