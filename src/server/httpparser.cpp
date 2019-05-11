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

	//Last thing we read was \r\n to skip those two bytes
	start+=2;
	//Check if there are bytes left for the body or if it was a header only request
	if(start<bytes)
	{
		//Calculate body start and size
		_body = &asyncReadBuf[start];
		_bodySize = bytes-start;
	}
	else
	{
		//No body
		_body = nullptr;
		_bodySize = 0;
	}

	//Start parsing the url encoded query parameters
	start = 0;
	//Search the beginning of the query parameters
	end = _url.find("?",start);

	if(end!=std::string::npos)
	{
		//Path is the part of the path before the question mark eg
		// /search/html?mark=x&quest=y
		// /search/html is the path and mark=x&quest=y the query
		_path = _url.substr(0,end);
		_query = _url.substr(end+1);
		//If we got a query parse the query
		if(_query!="")
			parse_query(_queryMap,_query);
	}
	else
	{
		//No question mark means path equals url and there is no query
		_path = _url;
		_query = "";
	}

	//Everything good seems like the message was a good one
	_healthy = true;
}

void http_request::print_request()
{
	//If the message is not healthy there is no reason to print it.
	if(!_healthy)
	{
		std::cout<<"Some corrupted message received"<<std::endl;
		return;
	}

	//Prints everything we got
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

const std::string_view &http_request::GetHeaders(const std::string key)
{
	return _headers[key];
}

const std::string_view http_request::GetQueryParams(const std::string key)
{
	return _queryMap[key];
}

const std::string_view &http_request::GetMethod()
{
	return _method;
}

const std::string_view &http_request::GetURL()
{
	return _url;
}

const std::string_view &http_request::GetPath()
{
	return _path;
}

const std::string_view &http_request::GetQuery()
{
	return _query;
}

bool http_request::IsHealthy()
{
	return _healthy;
}

const void *http_request::GetBody()
{
	return _body;
}

unsigned long http_request::GetBodySize()
{
	return _bodySize;
}


unsigned char tochar(char hi, char lo)
{
	//Convert hi to a decimal value F=15,E=14,D=13,...,9,8,7,6,5,4,3,2,1
	if(hi>='A'&&hi<='F')
		hi-=55;
	else if(hi>='a'&&hi<='f')
		hi-=('a'+10);
	else if(hi>='0'&&hi<='9')
		hi-=48;

	//Same for lo F=15,E=14,D=13,...,9,8,7,6,5,4,3,2,1
	if(lo>='A'&&lo<='F')
		lo-=55;
	else if(lo>='a'&&lo<='f')
		lo-=('a'+10);
	else if(lo>='0'&&lo<='9')
		lo-=48;
	//Calculate decimal value out of it
	return hi*16+lo;
}

/**
 * \todo Fix multiple key collisions and find a way to manage that eg collections=RXBADE&collections=RXFGABAAD
 */
void parse_query(std::map<std::string,std::string> &queryMap,std::string_view const &query)
{
	//Fill a map with the corresponding key: value pair and parse the url format
	std::string name = "";
	std::string value = "";
	//Set the current write pointer to the name as first comes the key then the value
	std::string *writer = &name;

	//Parse the whole string
	for(unsigned long i = 0; i < query.length(); i++)
	{
		//If there is a '%' sign it means the next two characters are hexadecimal format.
		//eg %20 = 32 = ' ' or %30 = 48 = '0' etc.
		if(query[i] == '%')
		{
			//Check if there is enough space left for conversion
			if(i+2<query.length())
			{
				(*writer)+=tochar(query[i+1],query[i+2]);
				i+=2;
			}
			else
				(*writer)+=query[i]; //If not just fill the normal % sign
		}
		else if(query[i] == '+') //+ means whitespace in url
		{
			(*writer)+=' ';
			continue;
		}
		else if(query[i] == '=') // '=' means the key is over and the value pair will start now eg. key=value
		{
			//If we are currently writing the key pair set the pointer to write the value pair now,
			//if we are in the value pair just put down the '=' sign in the value part 
			if(writer==&name)
			{
				writer = &value;
				continue;
			}
		}
		else if(query[i]=='&') //& means new key=value pair so write down the last pair and start the new pair
		{
			//No checking of existing name etc. if the user send stuff like that only the last key=value pair with the same key
			//will get saved
			queryMap[std::move(name)] = std::move(value);
			//Reset vars to start the next key=value pair
			name="";
			value="";
			writer = &name;
		}
		else
		{
			//No special character just take what the query got
			(*writer)+=query[i];
		}
	}
	//Write down the last key and value pair as there is no finishing & at the end of the url
	queryMap[std::move(name)] = std::move(value);
}


http_response::http_response()
{
}

http_response &http_response::status(const std::string hdr)
{
	_statusLine = hdr;
	return *this;
}

http_response &http_response::header(const std::string key, const std::string value)
{
	_headers[key] = value+"\r\n";
	return *this;
}

http_response &http_response::body(const std::string &bdy)
{
	_body += bdy;
	return *this;
}


void http_response::SendWithEOM(ssl_socket &sock)
{
	std::string *request = new std::string;
	*request = _statusLine;
	for(auto it = _headers.begin(); it!=_headers.end(); it++)
	{
		(*request)+=it->first;
		(*request)+=": ";
		(*request)+=it->second;
	}


	time_t t = time( NULL );
  	struct tm today = *localtime( &t );

	(*request)+="Content-Length: ";
	(*request)+=std::to_string(_body.length());
	(*request)+="\r\n";
	(*request)+="Server: clas-digital v0.1\r\n";
	(*request)+="Date: ";
	(*request)+=asctime(&today);
	(*request)+="\r\n";
	(*request)+=_body;
/*
	std::vector<boost::asio::const_buffer> scatter_gather_io;
	scatter_gather_io.push_back(boost::asio::buffer(_statusLine.c_str(),_statusLine.length()));
	scatter_gather_io.push_back(boost::asio::buffer(_body.c_str(),_body.length()));
*/
	boost::asio::async_write(sock,boost::asio::buffer(request->c_str(),request->length()),boost::bind(&http_response::handle_write_done,this,request,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
}

void http_response::handle_write_done(std::string *request,const boost::system::error_code &err, std::size_t bytes_transfered)
{
	if(err)
	{
		std::cout<<"Async write throwed error!"<<std::endl;
	}
	std::cout<<bytes_transfered<<" bytes send by async write"<<std::endl;
	delete request;
}
