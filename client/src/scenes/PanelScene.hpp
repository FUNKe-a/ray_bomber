#pragma once

#include "Scene.hpp"

struct Rectangle;

class PanelScene : public Scene
{
public:
    void render() const override final;

protected:
    virtual Rectangle panelBounds() const = 0;
    virtual void drawContent(Rectangle panel) const = 0;
};