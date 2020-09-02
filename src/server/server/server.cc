#include "server.hpp"
#include <mutex>



CLASServer &CLASServer::GetInstance()
{
  static CLASServer server;
  return server;
}


void CLASServer::Start(std::string listenAddress, int startPort)
{
  server_.listen(listenAddress.c_str(),startPort);
  Status(StatusBits::SERVER_STARTED,true);
}



void CLASServer::Status(StatusBits bit, bool value)
{
  status_.set((int)bit,value);
}

bool CLASServer::Status(StatusBits bit)
{
  return status_[(int)bit];
}

void CLASServer::Stop()
{
  server_.stop();
  Status(StatusBits::SERVER_STARTED,false);
}

CLASServer::CLASServer()
{
  status_.reset();
  startPort_ = 1409;
}
