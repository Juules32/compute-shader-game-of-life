#pragma once

#include <vector>
#include "ituGL/texture/Texture2DObject.h"

const auto ALIVE = std::byte{255};
const auto DEAD = std::byte{0};

class GameOfLife {
protected:
    int width = 0;
    int height = 0;
    bool isWrapping = true;
    bool isTrailing = true;

    std::vector<std::byte> GenerateGrid(bool randomGridGeneration);

public:
    virtual ~GameOfLife() = default;
    virtual void Initialize(int width, int height, bool randomGridGeneration) = 0;
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

enum GameOfLifeImplementation {
    CPU,
    GPU,
};
