add_library(networking)
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
