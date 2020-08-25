cmake_minimum_required(VERSION 2.8)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX /etc/clas-digital CACHE PATH whatever FORCE)
endif()

project(clas-digital)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
conan_basic_setup()  # Prepares the CMakeList.txt for Conan.


include_directories(
  "src"
  "deps")
set(CMAKE_CXX_FLAGS "-O3 -Wall -Werror -Wpedantic -std=c++17")


find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
include_directories(${Tesseract_INCLUDE_DIRS})

set(SRC_FILES_BOOKS src/books/Book.cpp src/books/Bookmanager.cpp src/books/metadata_handler.cc src/books/Search.cpp src/books/SearchOptions.cpp src/books/func.cpp src/books/fuzzy.cpp)
set(SRC_FILES src/main.cpp src/login/user.cpp src/login/usertable.cpp src/util/URLParser.cpp src/zotero/zotero.cpp src/ocr/tesseract.cpp src/util/ubuntu20matherror.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
add_executable(books.o EXCLUDE_FROM_ALL ${SRC_FILES_BOOKS} src/books/main.cpp)
add_executable(clas-digital.o ${SRC_FILES} ${SRC_FILES_BOOKS})
message("Libs: ${CONAN_LIBS}")  
target_link_libraries(clas-digital.o stdc++fs ${CONAN_LIBS})
target_link_libraries(books.o stdc++fs ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if(STABLE_INSTALL) 
	message("Installing to the stable version!")
else()
	message("Installing to development version")
endif()

if(NOT STABLE_INSTALL)
    set(INSTALL_DIRS /etc/clas-digital-devel)
    set(SERVICE_TARGET clas-digital-devel)
else()
    set(INSTALL_DIRS /etc/clas-digital)
    set(SERVICE_TARGET clas-digital)
endif()

ADD_CUSTOM_TARGET(install_stable
  COMMAND ${CMAKE_COMMAND} -DSTABLE_INSTALL=TRUE ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
  COMMENT "Installing to stable version"
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install
  COMMAND rm ${CMAKE_BINARY_DIR}/CMakeCache.txt
  COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}
)

install(FILES web/books/metadata_template.html DESTINATION ${INSTALL_DIRS}/web/books)
install(FILES web/books/pages_template.html DESTINATION ${INSTALL_DIRS}/web/books)
install(FILES src/clas-digital-devel.service DESTINATION /etc/systemd/system)
install(FILES src/default DESTINATION /etc/nginx/sites-available)
install(PROGRAMS bin/clas-digital.o DESTINATION ${INSTALL_DIRS})
install(DIRECTORY web DESTINATION ${INSTALL_DIRS} PATTERN "web/books/*" EXCLUDE)
install(DIRECTORY bin/trainingdata DESTINATION ${INSTALL_DIRS})
install(CODE "MESSAGE(\"Installing files done. Restarting nginx and the service ${SERVICE_TARGET}\")")
install(CODE "execute_process(COMMAND systemctl daemon-reload COMMAND systemctl restart nginx COMMAND systemctl restart ${SERVICE_TARGET})")
