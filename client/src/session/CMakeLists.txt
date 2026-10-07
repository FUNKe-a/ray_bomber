add_library(game_session)

target_sources(game_session
  PRIVATE
    GameSession.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      GameSession.hpp
)

target_link_libraries(game_session
  PUBLIC
    networking
    game_logic
)