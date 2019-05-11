cmake_minimum_required(VERSION 2.6)
project(clas-digital)

set(CMAKE_CXX_FLAGS "-O3 -W -Wall -Werror -std=c++1z")
set(SRC_FILES src/main.cpp src/server/server.cpp src/server/httpparser.cpp)

set(EXECUTABLE_OUTPUT_PATH ../bin) 
add_executable(clas-digital.o ${SRC_FILES})
target_link_libraries(clas-digital.o boost_system boost_thread ssl crypto)

