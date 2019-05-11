#include "httpparser.hpp"

http_request::http_request(const char *asyncReadBuf,size_t bytes)
{
	//The message is by default faulty at first, so there wont be any half parsed message
	//stating its healthy
	_healthy = false;

	//Create a string view to the whole message for find, substr etc.
	std::string_view theMessage = std::string_view(asyncReadBuf,bytes);

	unsigned long start = 0;
	unsigned long end = theMessage.find(" ");
	if(end==std::string::npos)
		return;

	//The header always looks like "GET /url HTTP/1.1\r\n"
	//So extract the method part of the message
	_method = theMessage.substr(start,end-start);

	start=end+1;
	end = theMessage.find(" ",start);
	if(end==std::string::npos)
		return;

	//Extract now the url part of the message
	_url = theMessage.substr(start,end-start);

	start=end+1;
	end=theMessage.find("\r\n",start);
	if(end==std::string::npos)
		return;

	//Just checking we are talking HTTP/1.1 here
	std::string_view check = theMessage.substr(start,end-start);
	if(check!="HTTP/1.1")
		return;

	//Because the line ended with /r/n we need to skip 2 bytes
	start = end+2;

	//Start the loop now to parse all http headers all http headers are in the format:
	//"header: value\r\n" The header part is finished by putting an empty line in after the headers
	//e. g. "header: value\r\n\r\n"
	//Ensuring there is out of bound reading in the loop
	for(;start<bytes;)
	{
		//Search the : in the new line to extract header and value
		end = theMessage.find(":",start);
		if(end==std::string::npos)
		{
			//if there is no ':' we probably reached the end of the http headers therefore checking if we got a
			//empty line here, and check also if there wont be an out of bounds read in the array
			if(((start+1)<bytes)&&(asyncReadBuf[start]=='\r')&&(asyncReadBuf[start+1]=='\n'))
				break;	//Ok end of header reached finish the loop
			return; //Error something is not right here
		}

		unsigned long hdrStart = start; //The start index of the header
		unsigned long hdrEnd = end;		//The end index of the header
		start = end+1;	//skip the ':'

		//Skip all trailing whitespaces, as http allows infinite white spaces before the header value starts
		for(;start<bytes;start++)
			if(asyncReadBuf[start]!=' ')
				break;

		//Search the end of the line to determine the end of the value
		end=theMessage.find("\r\n",start);

		if(end==std::string::npos)
			return;	//This is not a valid http message if there is no newline

		//Write the ["header"] = value to the maps
		_headers[theMessage.substr(hdrStart,hdrEnd-hdrStart)] = theMessage.substr(start,end-start);
		start=end+2;
	}

	start+=2;
	if(start<bytes)
	{
		_body = &asyncReadBuf[start];
		_bodySize = bytes-start;
	}
	else
	{
		_body = nullptr;
		_bodySize = 0;
	}
	start = 0;
	end = _url.find("?",start);
	if(end!=std::string::npos)
	{
		_path = _url.substr(0,end);
		_query = _url.substr(end+1);
		if(_query!="")
			parse_query(_queryMap,_query);
	}
	else
	{
		_path = _url;
		_query = "";
	}

	_healthy = true;
}

void http_request::print_request()
{
	if(!_healthy)
	{
		std::cout<<"Some corrupted message received"<<std::endl;
		return;
	}
	std::cout<<"Method: "<<_method<<std::endl;
	std::cout<<"URL: "<<_url<<std::endl;
	std::cout<<"Path: "<<_path<<std::endl;
	std::cout<<"Query: "<<_query<<std::endl;
	std::cout<<"Query map: "<<std::endl;
	for(auto it = _queryMap.begin(); it != _queryMap.end(); it++)
		std::cout<<it->first<<":"<<it->second<<std::endl;
	std::cout<<"Headers: "<<std::endl;
	for(auto it = _headers.begin(); it != _headers.end(); it++)
		std::cout<<it->first<<":"<<it->second<<std::endl;

	std::cout<<"Body size: "<<_bodySize<<std::endl;
}

const std::string_view &GetHeaders(std::string key)
{
	return _headers[key];
}

const std::string_view &GetQueryParams(std::string key)
{
	return _queryMap[key];
}

const std::string_view &GetMethod()
{
	return _method;
}

const std::string_view &GetURL()
{
	return _url;
}

const std::string_view &GetPath()
{
	return _path;
}

const std::string_view &GetQuery()
{
	return _query;
}

bool IsHealthy()
{
	return _healthy;
}

const void *GetBody()
{
	return _body;
}

unsigned long GetBodySize()
{
	return _bodySize;
}


unsigned char tochar(char hi, char lo)
{
	if(hi>='A'&&hi<='F')
		hi-=55;
	else if(hi>='a'&&hi<='f')
		hi-=('a'+10);
	else if(hi>='0'&&hi<='9')
		hi-=48;

	if(lo>='A'&&lo<='F')
		lo-=55;
	else if(lo>='a'&&lo<='f')
		lo-=('a'+10);
	else if(lo>='0'&&lo<='9')
		lo-=48;
	return hi*16+lo;
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
	queryMap[std::move(name)] = std::move(value);
}
