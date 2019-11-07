cmake_minimum_required(VERSION 2.6)
project(clas-digital)

include_directories(
  "src"
  "deps")
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -Werror -Wpedantic -march=native -std=c++1z")


find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

set(SRC_FILES src/main.cpp src/login/user.cpp src/login/usertable.cpp src/util/URLParser.cpp src/zotero/zotero.cpp)
set(SRC_FILES_BOOKS src/books/Book.cpp src/books/Bookmanager.cpp src/books/Metadata.cpp src/books/Search.cpp src/books/SearchOptions.cpp src/books/func.cpp src/books/fuzzy.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
add_executable(books.o EXCLUDE_FROM_ALL ${SRC_FILES_BOOKS} src/books/main.cpp)
add_executable(clas-digital.o ${SRC_FILES} ${SRC_FILES_BOOKS})
target_link_libraries(clas-digital.o stdc++fs ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(books.o stdc++fs ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


install(FILES src/clas-digital.service DESTINATION /etc/systemd/system)
install(FILES bin/cert.pem bin/key.pem DESTINATION /etc/clas-digital/bin)
install(FILES src/default DESTINATION /etc/nginx/sites-available)
install(PROGRAMS bin/clas-digital.o DESTINATION /etc/clas-digital)
install(DIRECTORY web DESTINATION /etc/clas-digital PATTERN "web/books/*" EXCLUDE)
install(CODE "execute_process(COMMAND systemctl daemon-reload COMMAND systemctl restart nginx COMMAND systemctl restart clas-digital)")
