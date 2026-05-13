#include "LifeSimulation.hpp"
#include <random>

void LifeSimulation::SetWrapping(bool value) {
    isWrapping = value;
}

bool LifeSimulation::GetWrapping() {
    return isWrapping;
}

void LifeSimulation::SetTrailing(bool value) {
    isTrailing = value;
}

bool LifeSimulation::GetTrailing() {
    return isTrailing;
}

std::vector<std::byte> LifeSimulation::GenerateGrid(bool randomGridGeneration) {
    std::vector<std::byte> grid(width * height * 2);

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 1);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            size_t index = (y * width + x) * 2;
            bool alive = randomGridGeneration ? dist(rng) : false;
            grid[index + 0] = alive ? ALIVE : DEAD; // R channel
            grid[index + 1] = alive ? ALIVE : DEAD; // G channel
        }
    }

    return grid;
}

int LifeSimulation::GetWidth() {
    return width;
}

int LifeSimulation::GetHeight() {
    return height;
}
