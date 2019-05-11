#include "server.hpp"

/**@brief Handles a TCP Session uses async reads and writes to perform requests
 * The session uses asynchronous read and write operations to communicate with the client. Only used
 * for HTTPS clients at the moment may be used for other purposes later on.
 */
class session
{
	public:
		/**
		 * Creates a new TCP session and pushes the work into the io_service.
		 * @param io_service The io_service to read from and write to
		 * @param context The ssl context used to encrypt the connection
		 */
		session(boost::asio::io_service& io_service, boost::asio::ssl::context& context);

		/**
		 * Returns the implementation of the socket used for system specific functions and APIs.
		 * @return socket implementation
		 */
		ssl_socket::lowest_layer_type& socket();

		/**
		 * Asynchronous starting the handshake.
		 */
		void start();

		/**
		 * The asynchronous called function that handles the ssl handshake and startes the first asynchronous read on the connection
		 * @param error The error returned by the system if the handshake fails.
		 */
		void handle_handshake(const boost::system::error_code& error);

		/**
		 * Asynchronous read operation used to read data from the client.
		 * @param error The error returned from the read function
		 * @param bytes_transferred The number of bytes transferred into the buffer
		 */
		void handle_read(const boost::system::error_code& error,
				size_t bytes_transferred);

	private:
		ssl_socket socket_; ///< The socket used to send encrypted data to the client
		constexpr unsigned long max_length = 1024*64;	///<The length of the buffer
		char data_[max_length];							///<The buffer to store the received data in
};


session::session(boost::asio::io_service& io_service, boost::asio::ssl::context& context)
	: socket_(io_service, context)
{
}

ssl_socket::lowest_layer_type& session::socket()
{
	return socket_.lowest_layer();
}

void session::start()
{
	socket_.async_handshake(boost::asio::ssl::stream_base::server,
			boost::bind(&session::handle_handshake, this,
				boost::asio::placeholders::error));
}

void session::handle_handshake(const boost::system::error_code& error)
{
	if (!error)
	{
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
				boost::bind(&session::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		delete this;
	}
}

void session::handle_read(const boost::system::error_code& error,
		size_t /*bytes_transferred*/)
{
	if (!error)
	{
		std::cout<<"RECEIVED BY THREAD: "<<boost::this_thread::get_id()<<std::endl;
		std::cout<<"RECEIVED BY: "<<this<<std::endl;
		std::cout<<data_<<std::endl;
		socket_.async_read_some(boost::asio::buffer(data_,max_length),
				boost::bind(&session::handle_read,this,boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		delete this;
	}
}


server::server(unsigned short port,const char *cert,const char *key)
	: io_service_(),acceptor_(io_service_,
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	context_(io_service_, boost::asio::ssl::context::sslv23)
{
	context_.set_options(
			boost::asio::ssl::context::default_workarounds
			| boost::asio::ssl::context::no_sslv2);
	context_.use_certificate_chain_file(cert);
	context_.use_private_key_file(key, boost::asio::ssl::context::pem);

	session* new_session = new session(io_service_, context_);
	acceptor_.async_accept(new_session->socket(),
			boost::bind(&server::handle_accept, this, new_session,
				boost::asio::placeholders::error));
}

void server::run(unsigned int threads)
{
	if(threads==0)
		threads = boost::thread::hardware_concurrency();

	std::cout<<"Running server with : "<<threads<<" Threads!"<<std::endl;
	std::cout<<"Thread 1 started!"<<std::endl;
	std::vector<boost::thread> workerThreads;
	std::mutex _lck;
	for(unsigned int i = 2; i < boost::thread::hardware_concurrency()+1; i++)
	{
		workerThreads.push_back(boost::thread([=,&_lck](){_lck.lock();std::cout<<"Thread "<<i<<" started!"<<std::endl;_lck.unlock();io_service_.run();}));
	}
	io_service_.run();
	for(auto &x : workerThreads)
		x.join();

}

void server::handle_accept(session* new_session,
		const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->start();
		new_session = new session(io_service_, context_);
		acceptor_.async_accept(new_session->socket(),
				boost::bind(&server::handle_accept, this, new_session,
					boost::asio::placeholders::error));
	}
	else
	{
		delete new_session;
	}
}
