#pragma once

#include <vector>
#include "ituGL/texture/Texture2DObject.h"

const auto ALIVE = std::byte{255};
const auto DEAD = std::byte{0};

const bool INITIAL_IS_WRAPPING = true;
const bool INITIAL_IS_TRAILING = true;

class LifeSimulation {
protected:
    int width = 0;
    int height = 0;
    bool isWrapping = INITIAL_IS_WRAPPING;
    bool isTrailing = INITIAL_IS_TRAILING;

    std::vector<std::byte> GenerateGrid(bool randomGridGeneration);

public:
    virtual ~LifeSimulation() = default;
    virtual void Initialize(
        int width,
        int height,
        bool randomGridGeneration,
        bool isWrapping,
        bool isTrailing
    ) = 0;
    virtual void Step() = 0;
    virtual void SetCell(int x, int y, bool alive) = 0;
    virtual bool GetCell(int x, int y) = 0;
    virtual Texture2DObject& GetTexture() = 0;
    virtual void SetTrailing(bool value);
    virtual bool GetTrailing();
    virtual void SetWrapping(bool value);
    virtual bool GetWrapping();

    int GetWidth();
    int GetHeight();
};

enum LifeImplementation {
    SingleThreaded,
    ComputeShader,
};
