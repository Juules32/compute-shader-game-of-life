#include "GameOfLife.hpp"

#include <random>

void GameOfLife::SetWrapping(bool value) {
    isWrapping = value;
}

bool GameOfLife::GetWrapping() {
    return isWrapping;
}

std::vector<std::byte> GameOfLife::GenerateGrid(bool randomGridGeneration) {
    std::vector<std::byte> grid(width * height, DEAD);

    if (randomGridGeneration) {
        // rand() makes patterns appear, so we use <random>
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution dist(0, 1);
        for (auto &cell : grid) {
            cell = dist(rng) ? ALIVE : DEAD;
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
