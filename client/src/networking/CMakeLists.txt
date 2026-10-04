add_library(networking)
target_compile_definitions(networking PUBLIC ASIO_STANDALONE)
if (WIN32)
  target_compile_definitions(networking PUBLIC _WIN32_WINNT=0x0A00 WIN32_LEAN_AND_MEAN)
  target_link_libraries(networking PUBLIC ws2_32 mswsock)
endif()
find_package(Threads REQUIRED)
target_link_libraries(networking PUBLIC Threads::Threads)
target_sources(networking
  PRIVATE
    Client.cpp

  PUBLIC
    FILE_SET HEADERS
	FILES
	  Client.hpp
)

target_include_directories(networking 
  PUBLIC
    "${FETCHCONTENT_BASE_DIR}/asio-src/include"
)

target_link_libraries(networking
  PUBLIC
    protocol
)
