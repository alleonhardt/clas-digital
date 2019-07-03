/**
 * @file src/server/URIObjects.hpp 
 *
 * Defines the basic URI Objects /search and /getprofileinfo etc.
 *
 */
#pragma once
#include "src/server/BasicHandlers.hpp"
#include "src/books/CBookManager.hpp"

/**
 * @brief Handles all read accesses to the user system
 */
class UserSystemHandler : public EmptyHandler
{
	public:

		/**
		 * @brief Sends back specific informations regarding the user profile and profile changes 
		 * @param headers The http request provided by the proxygen library
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

/**
 * @brief Handles all changes to the user table like create delete and change access
 */
class UpdateUserSystemHandler : public EmptyHandler
{
	public:
		/**
		 * @brief Only needs the body which contains the data to change the access rights and create the user.
		 * The supported actions are create delete and change user rights.
		 * @param body The body which contains an array of json with the commands that should be executed
		 * @return 200 Ok to the client if everything worked
		 */
		void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
};

/**
 * @brief Returns either all the books in the server or all the files in one book or a specific ressource from a specific book
 */
class GetBookRessource : public EmptyHandler
{
	public:
		/**
		 * @brief Sends back specific informations regarding the user profile and profile changes 
		 * @param headers The headers provided by the proxygen library
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

/**
 * @brief Handles the general search in all books
 */
class GetSearchHandler : public EmptyHandler
{
	public:
		/**
		 * @brief Returns an instance of the global book manager used to manage all books
		 *
		 * @return Returns the global book manager which manages all books
		 */
		static CBookManager &GetBookManager();
		/**
		 * @brief Tries to satify the search request and send back an json with all found books to the server
		 *
		 * @param headers The http message received from the the client with the parameters for the search
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

/**
 * @brief Searches a specific book for a specific word with the given fuzzyness 
 */
class GetSearchInBookHandler : public EmptyHandler
{
	public:
		/**
		 * @brief Searches in a specific book for a specififc word with the given fuzzynes and returns a json file with the results from the search
		 * @param[in] headers The headers for the http request received from the user
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

/**
 * @brief Redirects all request made to http port to the https website
 */
class RedirectToHTTPSHandler : public EmptyHandler
{
	public:
		/**
		 * @brief Sends back an redirect response no matter what the request is!
		 * @param[in] headers The http headers send from the user
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

class GetBookMetadata : public EmptyHandler
{
	public:
		/**
		 * @brief Returns the metadata for a specified book request must be like /getBookMetadata?book=ITEM_KEY
		 * @param headers The request headers send by the user
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

class UploadBookHandler : public EmptyHandler
{
	private:
		std::ofstream ofs;
		std::string _finalPath;
	public:
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
		void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
		void onEOM() noexcept override;
};

class StartSearch : public EmptyHandler
{
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
		void start(long long);
};

class RequestSearchProgress : public EmptyHandler
{
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};
