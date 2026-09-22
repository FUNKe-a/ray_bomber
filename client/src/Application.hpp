#pragma once

#include "Client.hpp"

#include <memory>

class ApplicationState;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    Client client;
    std::unique_ptr<ApplicationState> state;
};
