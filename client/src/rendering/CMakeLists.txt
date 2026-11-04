add_library(rendering)

target_sources(rendering
  PRIVATE
    GameRenderer.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      GameRenderer.hpp
)

target_link_libraries(rendering
  PRIVATE
    game_logic
    raylib
)