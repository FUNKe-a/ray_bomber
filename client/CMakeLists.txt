cmake_minimum_required(VERSION 4.4)

project(ray_bomber
  VERSION 0.1.0
  LANGUAGES CXX
)

include(FetchContent)

# Options
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)

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
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(
  raylib
  asio  
)

# Source file
set(RAY_BOMBER_SOURCES
  src/main.cpp

  src/game/Game.cpp
  src/game/Map.cpp
  src/game/Player.cpp

  src/networking/Client.cpp

  src/rendering/Renderer.cpp
)

set(RAY_BOMBER_HEADERS
  src/game/Game.hpp
  src/game/Map.hpp
  src/game/Player.hpp

  src/networking/Client.hpp

  src/rendering/Renderer.hpp
)


# Executable
add_executable(ray_bomber
  ${RAY_BOMBER_SOURCES}
  ${RAY_BOMBER_HEADERS}
)

target_compile_features(ray_bomber PRIVATE
  cxx_std_20
)

target_link_libraries(ray_bomber PRIVATE
  raylib
)

target_include_directories(ray_bomber PRIVATE
  ${asio_SOURCE_DIR}/asio/include
)

target_compile_definitions(ray_bomber PRIVATE
  ASIO_STANDALONE
  _WIN32_WINNT=0x0A00
  WIN32_LEAN_AND_MEAN
  NOGDI
  NOUSER
)

# Windows sockets
if (WIN32)
  target_link_libraries(ray_bomber PRIVATE
    ws2_32
  )
endif()

# Warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(ray_bomber PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(ray_bomber PRIVATE
    /W4
  )
endif()

# Output directories
set_target_properties(ray_bomber PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
)