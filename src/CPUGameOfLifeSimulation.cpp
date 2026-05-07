#include "CPUGameOfLifeSimulation.hpp"

#include <random>
#include <vector>

constexpr auto ALIVE = std::byte{0};
constexpr auto DEAD = std::byte{255};

void CPUGameOfLifeSimulation::Initialize(int width, int height) {
    Resize(width, height);

    // rand() makes patterns appear, so we use <random>
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1);
    for (auto &cell : grid) {
        cell = dist(rng) ? ALIVE : DEAD;
    }

    gridTexture.Bind();

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::MinFilter,
        GL_NEAREST
    );

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::MagFilter,
        GL_NEAREST
    );

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::WrapS,
        GL_CLAMP_TO_EDGE
    );

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::WrapT,
        GL_CLAMP_TO_EDGE
    );
}

void CPUGameOfLifeSimulation::Resize(int width, int height) {
    this->width = width;
    this->height = height;

    grid.resize(width * height, DEAD);

    UpdateTexture();
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

void CPUGameOfLifeSimulation::Update() {
    UpdateGameOfLife();
    UpdateTexture();
}

void CPUGameOfLifeSimulation::UpdateGameOfLife() {
    std::vector<std::byte> newGrid(width * height, DEAD);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            bool isSet = GetCell(x, y);
            int neighbors = CountNeighbors(x, y);
            int gridIndex = y * width + x;
            if (isSet) {
                if (neighbors < 2) {
                    newGrid[gridIndex] = DEAD;
                }
                if (neighbors == 2 || neighbors == 3) {
                    newGrid[gridIndex] = ALIVE;
                }
                if (neighbors > 3) {
                    newGrid[gridIndex] = DEAD;
                }
            } else {
                if (neighbors == 3) {
                    newGrid[gridIndex] = ALIVE;
                }
            }
        }
    }
    grid = std::move(newGrid);

    // BUG: Doesn't update??
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

bool CPUGameOfLifeSimulation::GetCell(int x, int y) {
    return grid[y * width + x] == ALIVE;
}

const Texture2DObject& CPUGameOfLifeSimulation::GetTexture() {
    return gridTexture;
}

void CPUGameOfLifeSimulation::SetCell(int x, int y, bool alive) {
    grid[y * width + x] = alive ? ALIVE : DEAD;
}

void CPUGameOfLifeSimulation::SetWrapping(bool value) {
    isWrapping = value;
}
