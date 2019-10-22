#include "util/URLParser.hpp"

char tonum(char ch)
{
	//Converts a character to the corresponding ascii int value
	//So Convert the ascii code for 0 eg 48 to "real zero"
	if( ch >= '0' && ch <= '9')
	{
		ch = ch-48;
	}
	else if( ch >= 'A' && ch <= 'F')
	{
		//Convert 'A' to 'F' to the values 10 to 15
		ch = ch-55;
	}
	else
	{
		//This is for errors, should never be executed
		ch = 0;
	}

	//Example 	tonum('9') = 9
	//Example2	tonum('F') = 15
	//return the constructed character
	return ch;
}

char tocharacter(char hi, char lo)
{
	//Convert 2 characters in Hexadecimal format to one ascii character
	//Example for 0x20 is tocharacter('2','0') = ' '
	//Example for 0x30 is tocharacter('3','0') = '0'
	return tonum(hi)*16+tonum(lo);
}

URLParser::URLParser(const std::string &fullURL)
{
	//no key and no value yet
	std::string key = "";
	std::string value = "";

	//Check if there are variables to parse
	//Variables definition in url always start with ?
	auto startPos = fullURL.find("?");
	if(startPos==std::string::npos) startPos = 0;
	else ++startPos;

	//First write to the key as the layout of https variables are
	//?key=value&key2=value2
	std::string *writeStr = &key;

	//If there are variables to parse, parse them
	if(startPos!=std::string::npos)
	{
		//Iterate over the remaining string to parse the vars
		for(unsigned long pos = startPos; pos < fullURL.length(); pos++)
		{
			//The current character is always the character at the currently
			//parsed position
			char nextChar = fullURL[pos];

			//If it is an 'equal' the key part is over and the variable part starts
			if(nextChar=='=')
			{
				//Write to the value string now
				writeStr = &value;
				continue;
			}
			//If an 'and' is found the key=var pair is finished so write the
			//key value pair to the map
			else if(nextChar=='&')
			{
				writeStr = &key;
				_urls[key] = std::move(value);

				//clear value and key pair to write the new pairs down
				value.clear();
				key.clear();
				continue;
			}

			//If there is an '%' in the URL string the next two characters decode
			//an forbitten character in hexadecimal for example
			//%30 means '0' or %20 means ' ' so if we find an % convert the next
			//two characters to one character in ascii format and move the read pointer
			//accordingly
			if(nextChar=='%'&&(pos+2)<fullURL.length())
			{
				nextChar = tocharacter(fullURL[pos+1],fullURL[pos+2]);
				pos+=2;
			}
			//+ denotes an whitespace in URL notation
			else if(nextChar=='+')
				nextChar=' ';

			//Write the parsed character either to the key or to the value string
			(*writeStr)+=nextChar;
		}
		//The last pair wont be saved because there is no finishing & so manually save the last
		//pair
		_urls[key]=value;
	}
}

unsigned long URLParser::size()
{
	return _urls.size();
}

const std::string &URLParser::operator[](std::string req)
{
	return _urls[req];
}

void URLParser::ShowALL()
{
	for(auto it = _urls.begin(); it != _urls.end(); it++)
	{
		std::cout<<it->first<<" mapped to "<<it->second<<std::endl;
	}
}

decltype(URLParser::_urls.begin()) URLParser::begin()
{
	return _urls.begin();
}

decltype(URLParser::_urls.end()) URLParser::end()
{
	return _urls.end();
}
