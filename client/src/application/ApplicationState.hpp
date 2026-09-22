#pragma once

#include <memory>

class ApplicationState
{
public:
    virtual ~ApplicationState() = default;

    // A null result keeps the current state. A non-null result becomes the
    // active state after the current frame has rendered.
    virtual std::unique_ptr<ApplicationState> update() = 0;
    virtual void render() const = 0;
};
