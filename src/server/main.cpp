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

  nlohmann::json root_user;
  root_user["email"] = "root@gmail.com";
  root_user["password"] = "123";
  root_user["access"] = 7;

  server.GetUserTable()->AddUser(root_user);
  auto err2 = server.Start("127.0.0.1");
  if(err2.GetErrorCode() != CLASServer::ReturnCodes::OK) {
    std::cout<<termcolor::red<<"Something went wrong..."<<std::endl;
    err2.print();
    return  0;
  }
  return 0;
}
