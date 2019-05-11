#pragma once
#include <string>
#include <vector>

class http_message
{
	public:
		http_message();
		http_message(std::string &&msg);

		http_message &header(const char *hdr,const char *val);
		http_message &body(const char *bdy);

		std::string operator()();
	private:
		std::map<std::string,std::string> _headers;
};
