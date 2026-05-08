#pragma once

#include "ituGL/texture/Texture2DObject.h"

constexpr auto ALIVE = std::byte{0};
constexpr auto DEAD = std::byte{255};

class GameOfLifeSimulation
{
protected:
    int width = 0;
    int height = 0;
    bool isWrapping = true;
public:
    virtual ~GameOfLifeSimulation() = default;
    virtual void Initialize(int width, int height) = 0;
    virtual void Update() = 0;
    virtual void Resize(int width, int height) = 0;
    virtual void SetCell(int x, int y, bool alive) = 0;
    virtual bool GetCell(int x, int y) = 0;
    virtual const Texture2DObject& GetTexture() = 0;
    virtual void SetWrapping(bool value) = 0;
    bool GetWrapping();
};

enum SimulationType {
    CPU,
    GPU,
};