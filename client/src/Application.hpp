#pragma once

#include "GameSession.hpp"
#include "GameFacade.hpp"
#include "ConnectionSettings.hpp"
#include "SceneFactory.hpp"

#include <memory>

class Scene;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    ConnectionSettings settings;
    GameSession session;
    GameFacade facade;
    SceneFactory factory;
    std::unique_ptr<Scene> scene;
};
