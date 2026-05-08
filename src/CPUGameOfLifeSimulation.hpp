#pragma once

#include "GameOfLifeSimulation.hpp"
#include <vector>

class CPUGameOfLifeSimulation : public GameOfLifeSimulation {
private:
    void Initialize(int width, int height, bool randomGridGeneration) override;
    void Update() override;
    void SetCell(int x, int y, bool alive) override;
    bool GetCell(int x, int y) override;
    const Texture2DObject& GetTexture() override;
    void SetWrapping(bool value) override;

    void UpdateTexture();
    void UpdateGameOfLife();
    int CountNeighbors(int x, int y);

    std::vector<std::byte> grid;
    Texture2DObject gridTexture;
};
