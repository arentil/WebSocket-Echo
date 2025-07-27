cmake_minimum_required(VERSION 4.1.0)

project(WebSocket-Echo VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  set(CMAKE_CXX_FLAGS "/std:c++23preview /Wall")
else()
  set(CMAKE_CXX_FLAGS "-std=c++23 -Wall -Wextra -pedantic -Wconversion -Wcast-align -Wunused -Wshadow -Wold-style-cast -lgcov")
endif ()

set(SERVER_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/server/main.cpp)
set(CLIENT_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/client/main.cpp)

add_executable(server ${SERVER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})