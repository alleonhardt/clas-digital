#include "server.hpp"
#include <mutex>



CLASServer &CLASServer::GetInstance()
{
  static CLASServer server;
  return server;
}


void CLASServer::Start(std::string listenAddress, int startPort)
{
  Status(StatusBits::SERVER_STARTED,true);
  server_.listen(listenAddress.c_str(),startPort);
}



void CLASServer::Status(StatusBits bit, bool value)
{
  std::lock_guard lck(exclusive_section_);
  status_.set((int)bit,value);
}

bool CLASServer::Status(StatusBits bit)
{
  std::lock_guard lck(exclusive_section_);
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