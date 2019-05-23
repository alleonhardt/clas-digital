cmake_minimum_required(VERSION 2.6)
project(clas-digital)

include_directories(
  "src"
  "."
  ${CURSES_INCLUDE_DIR})
set(CMAKE_CXX_FLAGS "-Wall -Werror -Wpedantic -std=c++1z")
#
# - Try to find Facebook proxygen library
# This will define
# PROXYFGEN_FOUND
# PROXYGEN_INCLUDE_DIR
# PROXYGEN_LIBRARIES
#

find_package(Folly REQUIRED)
find_package(Wangle REQUIRED)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(Fizz QUIET)
find_package(CURL REQUIRED)
find_package(Curses REQUIRED)
find_path(
  PROXYGEN_INCLUDE_DIR
  NAMES "proxygen/httpserver/HTTPServer.h"
  HINTS
    "/usr/local/facebook/include"
)

find_library(
  PROXYGEN_LIBRARY
  NAMES proxygenlib
  HINTS
    "/usr/local/facebook/lib"
  )

find_library(
  PROXYGEN_HTTP_SERVER
  NAMES proxygenhttpserver
  HINTS
    "/usr/local/facebook/lib"
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROXYGEN_LIBRARIES
  ${PROXYGEN_LIBRARY}
  ${PROXYGEN_HTTP_SERVER}
)

set(PROXYGEN_LIBRARIES ${PROXYGEN_LIBRARIES} ${FOLLY_LIBRARIES} ${WANGLE_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PROXYGEN DEFAULT_MSG PROXYGEN_INCLUDE_DIR PROXYGEN_LIBRARIES)

mark_as_advanced(PROXYGEN_INCLUDE_DIR PROXYGEN_LIBRARIES PROXYGEN_FOUND)

if(PROXYGEN_FOUND AND NOT PROXYGEN_FIND_QUIETLY)
  message(STATUS "PROXYGEN: ${PROXYGEN_INCLUDE_DIR}")
endif()


set(SRC_FILES src/server/GetHandler.cpp src/server/PostHandler.cpp src/login/user.cpp src/login/usertable.cpp src/server/URIObjects.cpp src/zotero/zotero.cpp src/books/Book.cpp src/books/Bookmanager.cpp src/books/Metadata.cpp src/books/SearchOptions.cpp src/books/func.cpp src/books/Search.cpp src/server/HandlerFactory.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)

add_executable(unittest.o ${SRC_FILES} src/console/unittestconsole.cpp src/test.cpp src/login/tests/TestUser.cpp src/login/tests/TestUsertable.cpp src/zotero/tests/TestZotero.cpp src/server/tests/TestURIFile.cpp)
add_custom_command(
	TARGET unittest.o
	POST_BUILD
	COMMAND ${EXECUTABLE_OUTPUT_PATH}/unittest.o
	WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
    COMMENT "Running unit tests..."
)

add_executable(clas-digital.o ${SRC_FILES} src/console/console.cpp src/main.cpp)
target_link_libraries(clas-digital.o ${PROXYGEN_LIBRARIES} ${CURL_LIBRARIES} stdc++fs ${CURSES_LIBRARY})
target_link_libraries(unittest.o ${PROXYGEN_LIBRARIES} ${GTEST_BOTH_LIBRARIES} ${CURL_LIBRARIES} stdc++fs ${CURSES_LIBRARY})
