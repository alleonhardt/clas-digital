#include "server/server.hpp"
#include <gtest/gtest.h>


TEST(CLASServer, Constructor) {
  CLASServer &srv = CLASServer::GetInstance();
  EXPECT_EQ(&srv, &CLASServer::GetInstance());
}


TEST(CLASServer, Start) {
  CLASServer &srv = CLASServer::GetInstance();
  EXPECT_EQ(srv.Status(CLASServer::StatusBits::SERVER_STARTED),false);
  EXPECT_EQ(srv.Status(CLASServer::StatusBits::GLOBAL_SHUTDOWN),false);
}
