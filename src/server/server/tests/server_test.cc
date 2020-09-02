#include "server/server.hpp"
#include <gtest/gtest.h>


TEST(CLASServer, Constructor) {
  CLASServer &srv = CLASServer::GetInstance();
  EXPECT_EQ(&srv, &CLASServer::GetInstance());
}


TEST(CLASServer, Start) {
  CLASServer &srv = CLASServer::GetInstance();
  srv.Start("localhost", 1409);
  EXPECT_EQ(srv.Status(CLASServer::StatusBits::SERVER_STARTED),true);

  srv.Stop();
  EXPECT_EQ(srv.Status(CLASServer::StatusBits::SERVER_STARTED),false);
}
