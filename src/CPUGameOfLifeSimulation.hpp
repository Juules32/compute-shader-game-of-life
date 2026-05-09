#pragma once

#include "GameOfLifeSimulation.hpp"
#include <vector>

class CPUGameOfLifeSimulation : public GameOfLifeSimulation {
public:
    void Initialize(int width, int height, bool randomGridGeneration) override;
    void Update() override;
    void SetCell(int x, int y, bool alive) override;
    bool GetCell(int x, int y) override;
    const Texture2DObject& GetTexture() override;

private:
    void UpdateGameOfLife();
    void UpdateTexture();
    int CountNeighbors(int x, int y);

    std::vector<std::byte> grid;
    Texture2DObject gridTexture;
};
