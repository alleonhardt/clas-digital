#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>
#include <mutex>

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

class session
{
public:
  session(boost::asio::io_service& io_service, boost::asio::ssl::context& context);

  ssl_socket::lowest_layer_type& socket();

  void start();

  void handle_handshake(const boost::system::error_code& error);

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);

private:
  ssl_socket socket_;
  enum { max_length = 1024*64 };
  char data_[max_length];
};

class server
{
public:
  server(unsigned short port,const char *cert,const char *key);

  void run(unsigned int threads=0);

  void handle_accept(session* new_session,
      const boost::system::error_code& error);

private:
boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ssl::context context_;
};


