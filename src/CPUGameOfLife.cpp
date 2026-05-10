#include "CPUGameOfLife.hpp"

#include <random>
#include <vector>
#include <cassert>

void CPUGameOfLife::Initialize(int width, int height, bool randomGridGeneration) {
    this->width = width;
    this->height = height;

    grid = GenerateGrid(randomGridGeneration);

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

void CPUGameOfLife::Step() {
    std::vector<std::byte> newGrid(width * height * 2, DEAD);

    const float decay = 0.95f;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            const int index = (y * width + x) * 2;

            const bool alive = GetCell(x, y);
            const int neighbors = CountNeighbors(x, y);

            // Normalized previous trail value (0.0-1.0)
            float prevTrail = static_cast<unsigned char>(grid[index + 1]) / 255.0f;

            bool newAlive = false;

            // Game of life rules
            if (alive)
            {
                newAlive = (neighbors == 2 || neighbors == 3);
            }
            else
            {
                newAlive = (neighbors == 3);
            }

            // Trail logic
            float trail;

            if (isTrailing) {
                trail = prevTrail * decay;
                if (newAlive)
                {
                    trail = 1.0f;
                }
            } else {
                trail = 0.0f;
            }

            // Write R (alive)
            newGrid[index + 0] = newAlive ? ALIVE : DEAD;

            // Write G (trail)
            newGrid[index + 1] = static_cast<std::byte>(trail * 255.0f);
        }
    }

    grid = std::move(newGrid);

    UpdateTexture();
}

void CPUGameOfLife::SetCell(int x, int y, bool alive) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);

    grid[(y * width + x) * 2] = alive ? ALIVE : DEAD;
    grid[(y * width + x) * 2 + 1] = alive ? ALIVE : DEAD;

    UpdateTexture();
}

bool CPUGameOfLife::GetCell(int x, int y) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);

    return grid[(y * width + x) * 2] == ALIVE;
}

Texture2DObject& CPUGameOfLife::GetTexture() {
    return gridTexture;
}

void CPUGameOfLife::UpdateTexture() {
    gridTexture.Bind();
    gridTexture.SetImage<std::byte>(
        0,
        width,
        height,
        TextureObject::FormatRG,
        TextureObject::InternalFormatRG8,
        std::span(grid),
        Data::Type::UByte
    );
}

int CPUGameOfLife::CountNeighbors(int x, int y) {
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
