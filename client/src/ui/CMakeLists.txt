add_library(ui)

target_sources(ui
  PRIVATE
    UI.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      UI.hpp
)

target_link_libraries(ui
  PUBLIC
    raylib
)