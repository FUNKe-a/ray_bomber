#pragma once

#include "Scene.hpp"

#include <map>
#include <memory>
#include <string>

class GameSession;
class GameFacade;
struct ConnectionSettings;

// Factory Method creator.
class SceneCreator
{
public:
    virtual ~SceneCreator() = default;

    virtual std::unique_ptr<Scene> create(std::string errorMessage) const = 0;
};

// Selects and owns the concrete creators.
class SceneFactory final
{
public:
    SceneFactory(
        GameSession& session,
        ConnectionSettings& settings,
        GameFacade& facade
    );

    // Creators retain references to this factory.
    SceneFactory(const SceneFactory&) = delete;
    SceneFactory& operator=(const SceneFactory&) = delete;
    SceneFactory(SceneFactory&&) = delete;
    SceneFactory& operator=(SceneFactory&&) = delete;

    std::unique_ptr<Scene> create(
        SceneType type,
        std::string errorMessage = ""
    ) const;

private:
    std::map<SceneType, std::unique_ptr<SceneCreator>> creators;
};