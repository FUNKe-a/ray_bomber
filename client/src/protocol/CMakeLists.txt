add_library(protocol Protocol.cpp)

target_sources(protocol 
  PUBLIC
    FILE_SET HEADERS
    FILES 
      Protocol.hpp
      Frame.hpp
)

target_link_libraries(protocol 
  PUBLIC
    proto_files
)
