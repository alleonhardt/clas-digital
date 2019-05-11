#pragma once
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

unsigned char tochar(char hi, char lo);



class http_request
{
	public:
		http_request();
		http_request(const char *asyncReadBuf,size_t bytes);


	private:
		std::string_view _method;
		std::string_view _url;
		std::string_view _path;
		std::string_view _query;
		std::map<std::string_view,std::string_view> _headers;
		bool _healthy;
		void *_body;
		unsigned long _bodySize;
};
