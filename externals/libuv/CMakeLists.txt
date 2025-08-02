cmake_minimum_required(VERSION 3.5)
include(FetchContent)
FetchContent_Declare(
  libuv
  GIT_REPOSITORY https://github.com/libuv/libuv
  GIT_TAG v1.51.0
  GIT_SHALLOW ON
  GIT_SUBMODULES ""
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(libuv)