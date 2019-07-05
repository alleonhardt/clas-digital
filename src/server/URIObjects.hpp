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

/**
 * @brief Searches a specific book for a specific word with the given fuzzyness 
 */
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

/**
 *
 * @brief Searches a specific book for a specific word with the given fuzzyness 
 */
class UploadBookHandler : public EmptyHandler
{
	private:
		std::ofstream ofs;  ///<The file handle to the file which is uploaded
		std::string _finalPath;	///< The final path to which the file is written to

	public:
		/**
		 * @brief handles the upload book request and tries to download the given files
		 *
		 * @param headers The specific header for the specific upload
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;

		/**
		 * @brief The function handles the upload of data to the server and puts the data in the right file
		 *
		 * @param body The data which is contained by the uploaded file
		 */
		void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

		/**
		 * @brief Finalizes the data given by the user. Could be unzipping or decrypting of data
		 */
		void onEOM() noexcept override;
};

/**
 * @brief Searches a specific book for a specific word with the given fuzzyness 
 */
class StartSearch : public EmptyHandler
{
	public:
		/**
		 * @brief Handles the start search request, this request starts a specific search registered before with /searchword
		 *
		 * @param headers The parameters given by the client with which we should start the search
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;

	private:
		/**
		 * @brief Dummy function used as thread entry point, this function will asynchronous search for the given parameters
		 *
		 * @param sid The search id, of the search to be started
		 * @param evb The event base from folly which is needed to execute ReponseBuilder on the right thread
		 */
		void start(long long sid,folly::EventBase *evb);
};

/**
 *
 * @brief Searches a specific book for a specific word with the given fuzzyness 
 */
class RequestSearchProgress : public EmptyHandler
{
	public:
		/**
		 * @brief Returns the search progress as JSON to the client, the headers must include the search id of the requested search if the search does not exist the server will return 1000
		 *
		 * @param headers The parameters given by the client packed in a http message
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};
