cmake_minimum_required(VERSION 3.5)
include(FetchContent)
FetchContent_Declare(
  ZLIB_content
  GIT_REPOSITORY https://github.com/madler/zlib
  GIT_TAG v1.3.1
)
FetchContent_MakeAvailable(ZLIB_content)
add_library(ZLIB::ZLIB ALIAS zlib)
target_include_directories(zlib INTERFACE ${zlib_content_SOURCE_DIR} ${zlib_content_BINARY_DIR})
set(ZLIB_BUILD_EXAMPLES FALSE)
set(ZLIB_BUILD_TESTING FALSE)
set(ZLIB_BUILD_MINIZIP FALSE)
set(ZLIB_FOUND ON PARENT_SCOPE)
set(ZLIB_LIBRARY ZLIB::ZLIB PARENT_SCOPE)
set(ZLIB_LIBRARIES ZLIB::ZLIB PARENT_SCOPE)
set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} PARENT_SCOPE)
set(ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} PARENT_SCOPE)