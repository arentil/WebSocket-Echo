cmake_minimum_required(VERSION 4.1.0)
include(FetchContent)

project(WebSocket-Echo VERSION 0.0.1 LANGUAGES C CXX)
# using C for external libraries



set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  set(CMAKE_CXX_FLAGS "/std:c++23preview /EHsc /bigobj")
  # /EHsc is needed for boost to generate boost::throw_exception handlers
else()
  set(CMAKE_CXX_FLAGS "-std=c++23 -Wall -Wextra -pedantic -Wconversion -Wcast-align -Wunused -Wshadow -Wold-style-cast -lgcov")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif ()


# ---------------------------------------------------------------
#        Externals: libuv, ZLIB, uSockets, uWebSockets, Boost
# ---------------------------------------------------------------
set(FETCHCONTENT_QUIET FALSE)
add_subdirectory(externals)

find_package(Qt6 REQUIRED COMPONENTS WebSockets Core Network)
# ===============================================================


# ------------------------------------------
#              WebSocket-Echo
# ------------------------------------------
add_executable(beast_client ${CMAKE_CURRENT_SOURCE_DIR}/client/beast_client.cpp)
add_executable(uwebsockets_server ${CMAKE_CURRENT_SOURCE_DIR}/server/uwebsockets_server.cpp)
add_executable(uwebsockets_workers_server ${CMAKE_CURRENT_SOURCE_DIR}/server/uwebsockets_workers_server.cpp)

target_link_libraries(beast_client PRIVATE Boost::asio Boost::beast)
target_link_libraries(uwebsockets_server PRIVATE uWebSockets)
target_link_libraries(uwebsockets_workers_server PRIVATE uWebSockets)

if (Qt6_FOUND)
  qt_standard_project_setup()
  qt_add_executable(qt_server ${CMAKE_CURRENT_SOURCE_DIR}/server/qt_server.cpp)
  target_link_libraries(qt_server PRIVATE Qt6::Core Qt6::Network Qt6::WebSockets)
endif()
# ===============================================================


# ------------------------------------------
#           POST BUILD COMMANDS
# ------------------------------------------
if (MSVC)
  # Copy generated ZLIB library into project output directory()
  add_custom_command(TARGET uwebsockets_server 
                    POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
                    copy "${zlib_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlibd.dll" 
                          "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
                    COMMENT "Copying ZLIB library into build folder")
endif()
# ===============================================================
