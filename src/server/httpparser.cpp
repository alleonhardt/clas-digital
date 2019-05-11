#include "httpparser.hpp"

http_request::http_request()
{}

http_request::http_request(const char *asyncReadBuf,size_t bytes)
{
	_healthy = false;
	std::string_view theMessage = std::string_view(asyncReadBuf,bytes);
	unsigned long start = 0;
	unsigned long end = theMessage.find(" ");
	if(end==std::string::npos)
		return;

	_method = theMessage.substr(start,end-start);

	start=end+1;
	end = theMessage.find(" ",start);
	if(end==std::string::npos)
		return;
	_url = theMessage.substr(start,end-start);

	start=end+1;
	end=theMessage.find("\r\n",start);
	if(end==std::string::npos)
		return;

	std::string_view check = theMessage.substr(start,end-start);
	if(check!="HTTP/1.1")
		return;

	start = end+2;
	for(;start<bytes;)
	{
		end = theMessage.find(":",start);
		if(end==std::string::npos)
		{
			if(((start+1)<bytes)&&(asyncReadBuf[start]=='\r')&&(asyncReadBuf[start+1]=='\n'))
				break;
			return;
		}
		unsigned long hdrStart = start;
		unsigned long hdrEnd = end;
		start = end+1;
		for(;start<bytes;start++)
			if(asyncReadBuf[start]!=' ')
				break;
		end=theMessage.find("\r\n",start);
		if(end==std::string::npos)
			return;
		_headers[theMessage.substr(hdrStart,hdrEnd-hdrStart)] = theMessage.substr(start,end-start);
		start=end+2;
	}

	start = 0;
	end = _url.find("?",start);
	if(end!=std::string::npos)
	{
		_path = _url.substr(0,end);
		_query = _url.substr(end+1);
	}
	else
	{
		_path = _url;
		_query = "";
	}

	std::cout<<"Method: "<<_method<<std::endl;
	std::cout<<"URL: "<<_url<<std::endl;
	std::cout<<"Path: "<<_path<<std::endl;
	std::cout<<"Query: "<<_query<<std::endl;
	std::cout<<"Headers: "<<std::endl;
	for(auto it = _headers.begin(); it != _headers.end(); it++)
		std::cout<<it->first<<": "<<it->second<<std::endl;
	_healthy = true;
}


unsigned char tochar(char hi, char lo)
{
	if(hi>='A'&&hi<='F')
		hi-=55;
	if(hi>='a'&&hi<='f')
		hi-='a';
	return lo;
}

void parse_query(std::map<std::string,std::string> &queryMap,std::string_view const &query)
{
	std::string name = "";
	std::string value = "";
	std::string *writer = &name;
	for(unsigned long i = 0; i < query.length(); i++)
	{
		if(query[i] == '%')
		{
			if(i+2<query.length())
			{
				(*writer)+=tochar(query[i+1],query[i+2]);
				i+=2;
			}
			else
				(*writer)+=query[i];
		}
		else if(query[i] == '+')
		{
			(*writer)+=' ';
			continue;
		}
		else if(query[i] == '=')
		{
			if(writer==&name)
			{
				writer = &value;
				continue;
			}
		}
		else if(query[i]=='&')
		{
			queryMap[std::move(name)] = std::move(value);
			name="";
			value="";
			writer = &name;
		}
		else
		{
			(*writer)+=query[i];
		}
	}
}
