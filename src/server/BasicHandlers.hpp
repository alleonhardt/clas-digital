/**
 * @file src/server/GetHandler.hpp
 *
 * Defines the interface to the Basic Get Handler which does a lot of the servers disk IO
 *
 */
#pragma once
#include <memory>
#include <folly/Memory.h>
#include <folly/File.h>
#include <proxygen/httpserver/RequestHandler.h>
#include "src/books/json.hpp"
#include "src/login/user_system.hpp"
#include "src/util/debug.hpp"

namespace proxygen {
	class ResponseHandler;
}


/**
 * @brief Small class used for setting the default empty method for every request handler
 *
 * This Handler is just used to provide an default empty method for the pure virtual class Request Handler, therefore
 * it does not do anything at all expect implementing these empty methods
 * Usage is as follows
 * @code
 * class MyRequestHandler : public EmptyHandler
 * {
 * 		public:
 *		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
 *			noexcept override;	//This is okay now just defining the function you use in your handler
 * 			
 * };
 * @endcode
 */
class EmptyHandler : public proxygen::RequestHandler {
	public:
		std::shared_ptr<User> _user; ///< The user this specific request is from
	public:
		/**
		 * @brief Dummy function for the proxygen virtual function onRequest
		 * @param headers The HTTP Message provided by proxygen
		 */
		virtual void onRequest(std::unique_ptr<proxygen::HTTPMessage>)
			noexcept override{}

		/**
		 * @brief Dummy empty handler for the on body proxygen virtual function
		 *
		 * @param body The body provided by proxygen for this message, can be called multiple times for the same request if there
		 * is a lot of data
		 *
		 */
		virtual void onBody(std::unique_ptr<folly::IOBuf>) noexcept override{}


		/**
		 * @brief Dummy empty handler for the proxygen function onUpgrade
		 *
		 * @param proto The new protocol to follow from there on
		 *
		 */
		virtual void onUpgrade(proxygen::UpgradeProtocol) noexcept override{}

		/**
		 * @brief The empty handler for the proxygen function requestComplete
		 */
		virtual void requestComplete() noexcept override{}

		/**
		 * @brief The dummy function for the proxygen function onError
		 *
		 * @param err The error that occured
		 */
		virtual void onError(proxygen::ProxygenError) noexcept override{}

		/**
		 * @brief The dummy function for the proxygen function inEgressPaused
		 *
		 */
		virtual void onEgressPaused() noexcept override{}

		/**
		 * @brief The dummy function for the proxygen function onEgressResumed
		 */
		virtual void onEgressResumed() noexcept override{}

		/**
		 * @brief The dummy function for the end of message function
		 */
		virtual void onEOM() noexcept override{}
};

/**
 * @brief Sends an 404 not found message to the client with the given message
 * @param rsp The downstream_ Response Builder ever RequestHandler has got
 * @param message The message to set the body to, the format of the body will always be html
 * @code
 * SendErrorNotFound(downstream_); //Can be used like this in every handler inheriting from proxygen::RequestHandler or EmptyHandler
 * SendErrorNotFound(downstream_, "<h1>My special error</h1>"); //Or specify a string to send a specific error message back
 * @endcode
 */
void SendErrorNotFound(proxygen::ResponseHandler *rsp, std::string message="<center><h1>Not found!</h1></center>");

/**
 * @brief Sends an 401 access denied message to the client with the given message
 * @param rsp The downstream_ Response Builder ever RequestHandler has got
 * @param message The message to set the body to, the format of the body will always be html
 * @code
 * SendAccessDenied(downstream_); //Can be used like this in every handler inheriting from proxygen::RequestHandler or EmptyHandler
 * SendAccessDenied(downstream_, "<h1>My special error</h1>"); //Or specify a string to send a specific error message back
 * @endcode
 */
void SendAccessDenied(proxygen::ResponseHandler *rsp, std::string message="<center><h1>Access denied</h1></center>");

/**
 * @brief The Basic Get Handler which does almost all of the server disk IO acesses
 *
 * Most of the function 
 *
 */
class GetHandler : public EmptyHandler {
	public:
		/**
		 * @brief Tries to satisfy a ressource request, do access right check and provide either an error response or the ressource response
		 *
		 * @param headers The HTTP headers for this request provided by proxygen
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};

/**
 * @brief The class contains basic information about the URI file
 */
class URIFile
{
	private:
		std::string _path;		///< The path the file points to
		int _access;			///< The access rights needed to access the file
		std::unique_ptr<folly::IOBuf> _fileBuf;	///< The buffer which holds the content of the files
		std::string _mimeType; ///<The mime type of the file which is to be loaded
	
		/**
		 * @brief Reloads the buffer after somehow the buffer is empty on getBufferReference() or getBuffer() call
		 */
		void loadBuffer();
	public:
		/**
		 * @brief The constructor tells the URI Object has which file path on the disk and the access rights needed to access it.
		 * @param path The path to the file to load
		 * @param accessRights The access rights needed to access the file, the accessRights must be a power of two!
		 */
		URIFile(std::string path, int accessRights=0);

		/**
		 * @brief Copy constructur, careful never ever use that!!! It was just created because std::unordered_map needs a copy constructor
		 *
		 * @param fl The file to create this file from
		 *
		 */
		URIFile(const URIFile &fl) : URIFile(fl.getPath(),fl._access){} 

		/**
		 * @brief The move constructor constructs the new object by moving all the data out of the other URIFile object
		 * @param mvConst The object to move the data away from
		 */
		URIFile(URIFile &&mvConst) : _path(std::move(mvConst._path)), _access(mvConst._access),
									_fileBuf(std::move(mvConst._fileBuf)),_mimeType(std::move(mvConst._mimeType)){} 
		/**
		 * @brief Performs an access check on this file with the given access rights
		 *
		 * @param acc The access rights trying to access the file, can be any positive integer
		 */
		bool doAccessCheck(int acc) const;

		/**
		 * @brief Returns a const reference to the path the file points to
		 *
		 * @return A const reference to the path the file points to
		 */
		const std::string &getPath() const;
		
		/**
		 * @brief Returns the mime type of the given file path in html reprensentation
		 * @code
		 * //This is how to use it
		 * URIFile file("web/index.html",0);
		 * file.getMimeType(); //Will be "text/html"
		 * @endcode
		 * @return A const reference to the detected mime type
		 */
		const std::string &getMimeType() const;

		/**
		 * @brief Returns a unqiue ptr to the clone of the IOBuf so that in can be send to the client in a timely manner
		 * @code
		 * URIFile file("web/index.html",0);
		 * ResponseBuilder(downstream_)
		 * 	.status(200,"Ok")
		 * 	.header("Content-Type",file.getMimeType())
		 * 	.body(file.getBuffer());	//Dont move the buffer as it would remove the data from the buffer inside the URIFile class
		 * @endcode
		 * @return A unique ptr to a clone of the IOBuf which holds the file data
		 */
		std::unique_ptr<folly::IOBuf> getBuffer();

		/**
		 * @brief This function is mainly used if one wants to move the content of the buffer to a file, eg. when the URI File object is short lived
		 * @code
		 * URIFile file("web/index.html",0);
		 * ResponseBuilder(downstream_)
		 * 	.status(200,"Ok")
		 * 	.header("Content-Type",file.getMimeType())
		 * 	.body(std::move(file.getBufferReference()));
		 * 	return;	//The URIFile object is short lived, no reason to copy the whole buffer so just move the loaded file buff away
		 * @endcode
		 * @return The reference to a buffer
		 */
		std::unique_ptr<folly::IOBuf> &getBufferReference();
};


/**
 * @brief Handles the basic posts to the server mainly does the login and not much else
 *
 */
class PostHandler : public EmptyHandler {
	private:
		bool _logout;
	public:
		/**
		 * @brief Proxygen callback for body data tries to parse user name and password from the data
		 *
		 * @param body The data send with the post request
		 */
		void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;


		/**
		 * @brief The Function determines if the user wants to login or logout and handles the request accordingly
		 *
		 * @param headers The http message passed by proxygen to our handler
		 */
		void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
			noexcept override;
};
