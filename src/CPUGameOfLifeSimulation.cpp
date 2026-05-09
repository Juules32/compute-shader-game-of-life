#include "CPUGameOfLifeSimulation.hpp"

#include <random>
#include <vector>
#include <cassert>

void CPUGameOfLifeSimulation::Initialize(int width, int height, bool randomGridGeneration) {
    this->width = width;
    this->height = height;

    grid = GenerateNewGrid(randomGridGeneration);

    UpdateTexture();

    gridTexture.Bind();
    gridTexture.SetParameter(
        TextureObject::ParameterEnum::MinFilter,
        GL_NEAREST
    );
    gridTexture.SetParameter(
        TextureObject::ParameterEnum::MagFilter,
        GL_NEAREST
    );

    // Fixes alignment for widths not divisible by 4
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void CPUGameOfLifeSimulation::Update() {
    UpdateGameOfLife();
    UpdateTexture();
}

void CPUGameOfLifeSimulation::SetCell(int x, int y, bool alive) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);

    grid[y * width + x] = alive ? ALIVE : DEAD;

    UpdateTexture();
}

bool CPUGameOfLifeSimulation::GetCell(int x, int y) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);

    return grid[y * width + x] == ALIVE;
}

const Texture2DObject& CPUGameOfLifeSimulation::GetTexture() {
    return gridTexture;
}

void CPUGameOfLifeSimulation::UpdateGameOfLife() {
    std::vector<std::byte> newGrid(width * height, DEAD);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            const bool isSet = GetCell(x, y);
            const int neighbors = CountNeighbors(x, y);
            const int gridIndex = y * width + x;
            if (isSet) {
                if (neighbors == 2 || neighbors == 3) {
                    newGrid[gridIndex] = ALIVE;
                }
            } else {
                if (neighbors == 3) {
                    newGrid[gridIndex] = ALIVE;
                }
            }
        }
    }
    grid = std::move(newGrid);
}

void CPUGameOfLifeSimulation::UpdateTexture() {
    gridTexture.Bind();
    gridTexture.SetImage<std::byte>(
        0,
        width,
        height,
        TextureObject::FormatR,
        TextureObject::InternalFormatR8,
        std::span(grid),
        Data::Type::UByte
    );
}

int CPUGameOfLifeSimulation::CountNeighbors(int x, int y) {
    static constexpr int offsets[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    int count = 0;
    for (const auto offset: offsets) {
        const int nx = x + offset[0];
        const int ny = y + offset[1];

        if (isWrapping) {
            if (GetCell((nx + width) % width, (ny + height) % height)) {
                count++;
            }
        } else {
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (GetCell(nx, ny)) {
                    count++;
                }
            }
        }
    }
    return count;
}
