add_library(game_logic)
target_sources(game_logic
  PRIVATE
    Game.cpp
    Map.cpp
    Player.cpp

  PUBLIC
    FILE_SET HEADERS
	FILES
	  Game.hpp
	  Map.hpp
	  Player.hpp
    GameTypes.hpp
    GameEvent.hpp
)