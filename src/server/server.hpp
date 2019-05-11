#pragma once
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>
#include <mutex>

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;


/**
 * @brief The Basic Multithreaded HTTPS Server handles all requests.
 */
class server
{
	public:
		/**
		 * Constructs the server from a given port a certificate file path and a key file path.
		 * \todo{Implement some more stuff}
		 * @param port The port to let the server listen to
		 * @param cert The certificate file path to open
		 * @param key The key file path to open
		 */
		server(unsigned short port,const char *cert,const char *key);

		/**
		 * Runs the server with the given number of threads, if 0 is specified runs on as many threads
		 * as there are cores in the system.
		 * @param threads The number of threads to run the server on.
		 */
		void run(unsigned int threads=0);

		/**
		 * The asynchronous handle accept callback, registers if there are new clients available and creates a new session from them
		 * @param new_session The new session to start.
		 * @param error The error which is maybe thrown by the system as a result of the accept operation
		 */
		void handle_accept(session* new_session,
				const boost::system::error_code& error);

	private:
		boost::asio::io_service io_service_; ///<The io_service to run all work reads and writes on.
		boost::asio::ip::tcp::acceptor acceptor_;	///< The acceptor used to accept new incoming tcp connections
		boost::asio::ssl::context context_;			///<The ssl context used to specify the details of the ssl encryption used in the connnections
};


