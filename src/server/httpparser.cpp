#include "httpparser.hpp"

http_message::http_message()
{}

http_message::http_message(std::string &&msg)
{}


http_message &http_message::header(const char *hdr,const char *val)
{
	_headers[hdr] = val;
}
