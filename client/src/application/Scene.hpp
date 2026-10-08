#pragma once

#include "SceneTransition.hpp"

#include <optional>

class Scene
{
public:
    virtual ~Scene() = default;

    virtual std::optional<SceneTransition> update() = 0;
    
    // Draws inside the frame opened by Application.
    virtual void render() const = 0;
};
