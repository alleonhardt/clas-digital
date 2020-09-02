cmake_minimum_required(VERSION 2.8)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX /etc/clas-digital CACHE PATH whatever FORCE)
endif()

project(clas-digital)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
conan_basic_setup()  # Prepares the CMakeList.txt for Conan.


find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 17)

set(SRC_FILES_SEARCH src/search/book_manager/Book.cpp src/search/book_manager/Bookmanager.cpp src/search/search/Search.cpp src/search/search/SearchOptions.cpp src/search/main.cpp src/shared/metadata_handler.cc src/shared/util/func.cpp src/shared/util/fuzzy.cpp)

add_library(clas_server_lib OBJECT src/shared/metadata_handler.cc
  src/shared/util/URLParser.cpp src/shared/util/ubuntu20matherror.cpp
  src/shared/util/func.cpp src/server/login/user.cpp
  src/server/login/usertable.cpp src/server/ocr/tesseract.cpp
  src/server/zotero/zotero.cpp src/server/server/server.cc)


set(SRC_FILES_SERVER_TEST src/server/test/main.cc
  src/server/zotero/tests/zotero.cc src/server/server/tests/server_test.cc)

add_executable(clas_digital_server.o $<TARGET_OBJECTS:clas_server_lib> src/server/main.cpp)
add_executable(clas_digital_server_test.o $<TARGET_OBJECTS:clas_server_lib>
  ${SRC_FILES_SERVER_TEST})

set(CXX_STDLIB "")


add_executable(clas_digital_search.o  ${SRC_FILES_SEARCH})

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
  set(CXX_STDLIB stdc++)
endif()

target_include_directories(clas_server_lib PUBLIC
  "src/shared"
  "src/shared/util"
  "src/server"
)

target_include_directories(clas_digital_server.o PUBLIC
  "src/shared"
  "src/shared/util"
  "src/server"
)

target_include_directories(clas_digital_server_test.o PUBLIC
  "src/shared"
  "src/shared/util"
  "src/server"
)

target_include_directories(clas_digital_search.o PUBLIC
  "src/shared"
  "src/shared/util"
  "src/search"
)

target_link_libraries(clas_digital_server.o ${CONAN_LIBS} ${CXX_STDLIB})
target_link_libraries(clas_digital_server_test.o ${CONAN_LIBS} ${CXX_STDLIB})
target_link_libraries(clas_digital_search.o ${CXX_STDLIB})

set(INSTALL_DIRS /etc/clas-digital-devel)
set(SERVICE_TARGET clas-digital-devel)

install(FILES web/books/metadata_template.html DESTINATION ${INSTALL_DIRS}/web/books)
install(FILES web/books/pages_template.html DESTINATION ${INSTALL_DIRS}/web/books)
install(FILES src/server/${SERVICE_TARGET}.service DESTINATION /etc/systemd/system)
install(FILES src/default DESTINATION /etc/nginx/sites-available)
install(PROGRAMS build/bin/clas_digital_server.o DESTINATION ${INSTALL_DIRS})
install(PROGRAMS build/bin/clas_digital_search.o DESTINATION ${INSTALL_DIRS})
install(DIRECTORY web DESTINATION ${INSTALL_DIRS} PATTERN "web/books/*" EXCLUDE)
install(DIRECTORY bin/trainingdata DESTINATION ${INSTALL_DIRS})
install(CODE "MESSAGE(\"Installing files done. Restarting nginx and the service ${SERVICE_TARGET}\")")
install(CODE "execute_process(COMMAND systemctl daemon-reload COMMAND systemctl restart nginx COMMAND systemctl restart ${SERVICE_TARGET})")
