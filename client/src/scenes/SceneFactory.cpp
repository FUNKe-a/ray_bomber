#include "SceneFactory.hpp"

#include "ConnectingScene.hpp"
#include "LobbyScene.hpp"
#include "MenuScene.hpp"
#include "SettingsScene.hpp"
#include "PlayingScene.hpp"

#include <stdexcept>
#include <utility>

namespace
{
    class MenuSceneCreator final : public SceneCreator
    {
    public:
        MenuSceneCreator(
            GameSession& session, ConnectionSettings& settings
        )
            : session(session), settings(settings)
        {
        }

        std::unique_ptr<Scene> create(std::string errorMessage) const override
        {
            return std::make_unique<MenuScene>(
                session, settings, std::move(errorMessage)
            );
        }

    private:
        GameSession& session;
        ConnectionSettings& settings;
    };

    class SettingsSceneCreator final : public SceneCreator
    {
    public:
        SettingsSceneCreator(ConnectionSettings& settings): settings(settings)
        {
        }

        std::unique_ptr<Scene> create(std::string) const override
        {
            return std::make_unique<SettingsScene>(settings);
        }

    private:
        ConnectionSettings& settings;
    };

    class ConnectingSceneCreator final : public SceneCreator
    {
    public:
        ConnectingSceneCreator(GameSession& session): session(session)
        {
        }

        std::unique_ptr<Scene> create(std::string) const override
        {
            return std::make_unique<ConnectingScene>(session);
        }

    private:
        GameSession& session;
    };

    class LobbySceneCreator final : public SceneCreator
    {
    public:
        LobbySceneCreator(GameSession& session): session(session)
        {
        }

        std::unique_ptr<Scene> create(std::string) const override
        {
            return std::make_unique<LobbyScene>(session);
        }

    private:
        GameSession& session;
    };

    class PlayingSceneCreator final : public SceneCreator
    {
    public:
        explicit PlayingSceneCreator(GameSession& session): session(session)
        {
        }

        std::unique_ptr<Scene> create(std::string) const override
        {
            return std::make_unique<PlayingScene>(session);
        }

    private:
        GameSession& session;
    };
}

SceneFactory::SceneFactory(
    GameSession& session,
    ConnectionSettings& settings
)
{
    creators.emplace(
        SceneType::Menu, std::make_unique<MenuSceneCreator>(session, settings)
    );

    creators.emplace(
        SceneType::Settings, std::make_unique<SettingsSceneCreator>(settings)
    );

    creators.emplace(
        SceneType::Connecting, std::make_unique<ConnectingSceneCreator>(session)
    );

    creators.emplace(
        SceneType::Lobby, std::make_unique<LobbySceneCreator>(session)
    );

    creators.emplace(
        SceneType::Playing, std::make_unique<PlayingSceneCreator>(session)
    );
}

std::unique_ptr<Scene> SceneFactory::create(
    SceneType type,
    std::string errorMessage
) const
{
    const auto creator = creators.find(type);

    if (creator == creators.end())
    {
        throw std::invalid_argument("Unknown scene type.");
    }

    return creator->second->create(std::move(errorMessage));
}