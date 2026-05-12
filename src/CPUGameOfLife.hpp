#pragma once

#include "GameOfLife.hpp"
#include <vector>

class CPUGameOfLife : public GameOfLife {
public:
    void Initialize(
        int width,
        int height,
        bool randomGridGeneration,
        bool isWrapping,
        bool isTrailing
    ) override;
    void Step() override;
    void SetCell(int x, int y, bool alive) override;
    bool GetCell(int x, int y) override;
    Texture2DObject& GetTexture() override;

private:
    void UpdateTexture();
    int CountNeighbors(int x, int y);

    std::vector<std::byte> grid;
    Texture2DObject gridTexture;
};
