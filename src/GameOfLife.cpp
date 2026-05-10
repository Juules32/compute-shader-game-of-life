#include "GameOfLife.hpp"

#include <random>

void GameOfLife::SetWrapping(bool value) {
    isWrapping = value;
}

bool GameOfLife::GetWrapping() {
    return isWrapping;
}

void GameOfLife::SetTrailing(bool value) {
    isTrailing = value;
}

bool GameOfLife::GetTrailing() {
    return isTrailing;
}

std::vector<std::byte> GameOfLife::GenerateGrid(bool randomGridGeneration) {
    std::vector<std::byte> grid(width * height * 2);

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 1);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            size_t index = (y * width + x) * 2;

            bool alive = randomGridGeneration ? dist(rng) : false;

            // R channel
            grid[index + 0] = alive ? ALIVE : DEAD;

            // G channel
            grid[index + 1] = alive ? ALIVE : DEAD;
        }
    }

    return grid;
}

int GameOfLife::GetWidth() {
    return width;
}

int GameOfLife::GetHeight() {
    return height;
}
