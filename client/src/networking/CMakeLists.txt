add_library(networking)

target_sources(networking
  PRIVATE
    Client.cpp
    ProtobufAdapter.cpp
    ProtobufAdapter.hpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      Client.hpp
)

target_compile_definitions(networking
  PUBLIC
    ASIO_STANDALONE
)

target_include_directories(networking
  PUBLIC
    "${FETCHCONTENT_BASE_DIR}/asio-src/include"
)

find_package(Threads REQUIRED)

target_link_libraries(networking
  PUBLIC
    game_logic
    protocol
    Threads::Threads
)

if(WIN32)
  target_compile_definitions(networking
    PUBLIC
      _WIN32_WINNT=0x0A00
      WIN32_LEAN_AND_MEAN
  )

  target_link_libraries(networking
    PUBLIC
      ws2_32
      mswsock
  )
endif()