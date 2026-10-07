#pragma once

#include <memory>

class Scene
{
public:
    virtual ~Scene() = default;

    // A null result keeps the current state.
    // A non-null result replaces it after update(), before rendering.
    virtual std::unique_ptr<Scene> update() = 0;
    
    // Draws inside the frame opened by Application.
    virtual void render() const = 0;
};
