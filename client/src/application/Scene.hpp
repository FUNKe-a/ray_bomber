#pragma once

#include <memory>

class Scene
{
public:
    virtual ~Scene() = default;

    // A null result keeps the current state. A non-null result becomes the
    // active state after the current frame has rendered.
    virtual std::unique_ptr<Scene> update() = 0;
    virtual void render() const = 0;
};
