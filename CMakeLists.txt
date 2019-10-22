cmake_minimum_required(VERSION 2.6)
project(clas-digital)

include_directories(
  "src"
  "deps")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wpedantic -std=c++1z")

find_package(Threads REQUIRED)

set(SRC_FILES src/main.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
add_executable(clas-digital.o ${SRC_FILES})
target_link_libraries(clas-digital.o stdc++fs ${CMAKE_THREAD_LIBS_INIT})


install(FILES src/clas-digital.service DESTINATION /etc/systemd/system)
install(FILES bin/cert.pem bin/key.pem DESTINATION /etc/clas-digital/bin)
install(FILES src/default DESTINATION /etc/nginx/sites-available)
install(PROGRAMS bin/clas-digital.o DESTINATION /etc/clas-digital)
install(DIRECTORY web DESTINATION /etc/clas-digital)
install(CODE "execute_process(COMMAND systemctl daemon-reload COMMAND systemctl restart nginx COMMAND systemctl restart clas-digital)")
