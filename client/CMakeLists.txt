cmake_minimum_required(VERSION 3.23)
project(ray_bomber)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(outputDir "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${outputDir}/archive")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${outputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${outputDir}")
