add_library(ui)

target_sources(ui
  PRIVATE
    UI.cpp
    Controls.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      UI.hpp
      Controls.hpp
)

target_link_libraries(ui
  PUBLIC
    raylib
)