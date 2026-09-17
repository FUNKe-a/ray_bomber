cmake_minimum_required(VERSION 4.3)

project(ray_bomber
  VERSION 0.1.0
  LANGUAGES CXX
)

include(FetchContent)

# Options
set(outputDir "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${outputDir}/archive")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${outputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${outputDir}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 6.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-38-2
  SOURCE_SUBDIR "placeholder"
)

FetchContent_MakeAvailable(
  raylib
  asio  
)

add_executable(ray_bomber)
target_sources(ray_bomber
  PRIVATE
    src/main.cpp
)

add_library(game_logic)
target_sources(game_logic
  PRIVATE
    src/game/Game.cpp
    src/game/Map.cpp
    src/game/Player.cpp

  PUBLIC
    FILE_SET HEADERS
	BASE_DIRS
	  src/game
	FILES
	  src/game/Game.hpp
	  src/game/Map.hpp
	  src/game/Player.hpp
)

add_library(networking)
target_sources(networking
  PRIVATE
    src/networking/Client.cpp

  PUBLIC
    FILE_SET HEADERS
	BASE_DIRS
	  src/networking
	FILES
	  src/networking/Client.hpp
)

add_library(rendering)
target_sources(rendering
  PRIVATE
  src/rendering/Renderer.cpp

  PUBLIC
    FILE_SET HEADERS
	BASE_DIRS
	  src/rendering
	FILES
	  src/rendering/Renderer.hpp
)

add_library(protocol)
target_sources(protocol
  PRIVATE
    src/protocol/Protocol.cpp

  PUBLIC
    FILE_SET HEADERS
  BASE_DIRS
    src/protocol
  FILES
    src/protocol/Protocol.hpp
)
target_include_directories(protocol
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol"
)

# different way to include asio because
# it's a header only library.
target_include_directories(networking 
  PUBLIC
    "${FETCHCONTENT_BASE_DIR}/asio-src/include"
)

target_link_libraries(game_logic
  PUBLIC
    protocol
)

target_link_libraries(rendering 
  PRIVATE
    raylib
  PUBLIC
    game_logic
)

target_link_libraries(ray_bomber
  PRIVATE
    raylib
    game_logic
    networking
    rendering
)

target_compile_definitions(ray_bomber PRIVATE
  ASIO_STANDALONE
  _WIN32_WINNT=0x0A00
  WIN32_LEAN_AND_MEAN
  NOGDI
  NOUSER
)
#
# # Windows sockets
# if (WIN32)
#   target_link_libraries(ray_bomber PRIVATE
#     ws2_32
#   )
# endif()
#
# # Warnings
# if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
#   target_compile_options(ray_bomber PRIVATE
#     -Wall
#     -Wextra
#     -Wpedantic
#   )
# elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#   target_compile_options(ray_bomber PRIVATE
#     /W4
#   )
# endif()
