add_executable(ray_bomber)

if (WIN32)
  target_compile_definitions(ray_bomber PRIVATE
    ASIO_STANDALONE
    _WIN32_WINNT=0x0A00
    WIN32_LEAN_AND_MEAN
    NOGDI
    NOUSER
  )
endif()

target_sources(ray_bomber
  PRIVATE
    Application.cpp
    main.cpp
    application/Scene.hpp
    scenes/ConnectingScene.cpp
    scenes/ConnectingScene.hpp
    scenes/MenuScene.cpp
    scenes/MenuScene.hpp
    scenes/PlayingScene.cpp
    scenes/PlayingScene.hpp
)

target_include_directories(ray_bomber
  PRIVATE
    application
    scenes
)

add_subdirectory(protocol)
add_subdirectory(networking)
add_subdirectory(game)

target_link_libraries(ray_bomber
  PRIVATE
    raylib
    game_logic
    networking
)


