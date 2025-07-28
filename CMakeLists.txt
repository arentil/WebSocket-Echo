cmake_minimum_required(VERSION 4.1.0)

include(FetchContent)

project(WebSocket-Echo VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  set(CMAKE_CXX_FLAGS "/std:c++23preview")
else()
  set(CMAKE_CXX_FLAGS "-std=c++23 -Wall -Wextra -pedantic -Wconversion -Wcast-align -Wunused -Wshadow -Wold-style-cast -lgcov")
endif ()

# ---------------------
#        Boost
# ---------------------
if(POLICY CMP0167)
   cmake_policy(SET CMP0167 OLD)
endif()
find_package(Boost 1.88.0 REQUIRED COMPONENTS filesystem system thread regex)
include_directories(${Boost_INCLUDE_DIRS})
# ==========================================


# ---------------------------------------------------------------
#        Externals: libuv, ZLIB, uSockets, uWebSockets
# ---------------------------------------------------------------
add_subdirectory(externals)
# ===============================================================


# set(CLIENT_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/client/main.cpp)
set(SERVER_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/server/main.cpp)

# add_executable(client ${CLIENT_SOURCE_FILES})
add_executable(server ${SERVER_SOURCE_FILES})

# target_link_libraries(client PUBLIC ${Boost_LIBRARIES} Websockets)

target_link_libraries(server PUBLIC ${Boost_LIBRARIES} ZLIB::ZLIB uWebSockets)

# ------------------------------------------
#           POST BUILD COMMANDS
# ------------------------------------------
add_custom_command(TARGET server 
                   POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
                   copy "${zlib_BINARY_DIR}/${CMAKE_BUILD_TYPE}/zlibd.dll" 
                        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
                   COMMENT "Copying ZLIB library into build folder")
