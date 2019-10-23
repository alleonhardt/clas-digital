#pragma once
#include <iostream>
#include <string>
#include <map>

class URLParser
{
	private:
		std::map<std::string,std::string> _urls; 	///<The parsed vars from the url mapped to their respective name

	public:
		/**
		 * Creates a new URL Parser object automatically tries to parse the given
		 * url string.
		 * @param fullURL The url string to parse the parameters from
		 */
		URLParser(const std::string &fullURL);

		/**
		 * Returns the value for the given variable if the variable is not defined
		 * in the url string return an empty string.
		 * @param req The variable name to finde the value to
		 * @return The value corresponding to the given variable name
		 */
		const std::string &operator[](std::string req);

		/**
		 * Returns an iterator to the begin of the map with variables used
		 * to iterate over all parsed variables in the url string
		 */
		decltype(_urls.begin()) begin();

		/**
		 * Returns an iterator to the end of the map with variables used to
		 * iterate over all parsed variables in the url string
		 */
		decltype(_urls.end()) end();

		/**
		 * Returns the amount of variables parsed from the url string
		 * @return The amount of variables parsed from the url string
		 */
		unsigned long size();

		/**
		 * A Debug function can show all variables mapped to their respective value
		 */
                void ShowALL();
};
