#include <iostream>
#include <streambuf>
#include <chrono>
#include <exception>
#include <string_view>
#include <filesystem>
#include <sstream>

#include <signal.h>
#include "server/server.hpp"



using namespace clas_digital;

int main(int argc, char **argv)
{
  CLASServer &server = CLASServer::GetInstance();
  auto err = server.InitialiseFromFile("server.config",":memory:");
  if(err.GetErrorCode() != CLASServer::ReturnCodes::OK)
  {
    std::cout<<termcolor::red<<"Something went wrong..."<<std::endl;
    err.print();
    return  0;
  }

  server.Start("0.0.0.0");

  return 0;
}
