cmake_minimum_required(VERSION 2.8)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX /etc/clas-digital CACHE PATH whatever FORCE)
endif()

project(clas-digital)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
conan_basic_setup()  # Prepares the CMakeList.txt for Conan.
find_package(Git)
find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 17)


execute_process(COMMAND
  "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the date of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the subject of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%s
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# generate version.cc
configure_file("${CMAKE_SOURCE_DIR}/src/shared/version.hpp.in"
  "${CMAKE_SOURCE_DIR}/src/shared/version.hpp" @ONLY)





set(SRC_FILES_SEARCH src/search/book_manager/Book.cpp src/search/book_manager/Bookmanager.cpp src/search/search/Search.cpp src/search/search/SearchOptions.cpp src/search/main.cpp src/shared/metadata_handler.cc src/shared/util/func.cpp src/shared/util/fuzzy.cpp)

add_library(clas_server_lib OBJECT src/shared/metadata_handler.cc
  src/shared/util/URLParser.cpp src/shared/util/ubuntu20matherror.cpp
  src/shared/util/func.cpp src/server/login/user.cpp
  src/server/zotero/zotero.cpp src/server/server/server.cc
  src/server/server/server_config.cpp
  src/server/plugins/EventManager.cpp
  src/server/plugins/PlugInManager.cpp
  src/server/filehandler/filehandler.cpp)


set(SRC_FILES_SERVER_TEST src/server/test/main.cc
  src/server/server/tests/server_test.cc src/server/zotero/tests/zotero.cc
  src/server/login/tests/users_test.cc
  src/server/server/tests/server_config_test.cc
  src/server/plugins/tests/event_manager_test.cpp
  src/server/plugins/tests/plug_in_manager_test.cpp
  src/server/cache/tests/cache_tests.cpp)

if(MSVC)
  add_library(test_plugin SHARED src/server/plugins/tests/test_plugin.cpp $<TARGET_OBJECTS:clas_server_lib>)
else()
  add_library(test_plugin SHARED src/server/plugins/tests/test_plugin.cpp)
endif()

add_executable(clas_digital_server.o $<TARGET_OBJECTS:clas_server_lib> src/server/main.cpp)
add_executable(clas_digital_server_test.o $<TARGET_OBJECTS:clas_server_lib>
  ${SRC_FILES_SERVER_TEST})

set(CXX_STDLIB "")


add_executable(clas_digital_search.o  ${SRC_FILES_SEARCH})

if(MSVC)
  add_compile_options(/W4 /WX)
  target_link_libraries(test_plugin ${CONAN_LIBS} ${CXX_STDLIB})
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)

  set(CXX_STDLIB stdc++)
endif()

if(APPLE)
  target_link_options(test_plugin PUBLIC -undefined dynamic_lookup)
ENDIF(APPLE)

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

target_include_directories(test_plugin PUBLIC
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
target_link_libraries(clas_digital_search.o ${CONAN_LIBS} ${CXX_STDLIB})

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
