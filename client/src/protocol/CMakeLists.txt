add_library(protocol)
target_sources(protocol
  PRIVATE
    Protocol.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      Protocol.hpp
)
