cmake_minimum_required(VERSION 4.3)

project(ray_bomber
  VERSION 0.1.0
  LANGUAGES CXX
)

find_package(Protobuf REQUIRED)

include(cmake/options.cmake)
include(cmake/fetch.cmake)
include(cmake/generate.cmake)

add_subdirectory(src)
