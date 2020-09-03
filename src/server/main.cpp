#define CPPHTTPLIB_THREAD_POOL_COUNT 8

#include <iostream>
#include <streambuf>
#include <httplib.h>
#include <chrono>
#include <exception>
#include <string_view>
#include <filesystem>
#include <sstream>

#include <signal.h>

#include "URLParser.hpp"
#include "zotero/zotero.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //Needed for image dimensions


using namespace httplib;

int main(int argc, char **argv)
{
  if (argc < 2)
    return 0;
  int startPort = std::stoi(argv[1]);
  std::cout << "Starting on port: " << startPort << std::endl;
  return 0;
}
