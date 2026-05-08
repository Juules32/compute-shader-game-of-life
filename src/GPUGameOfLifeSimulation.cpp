#include "GPUGameOfLifeSimulation.hpp"
#include "util.hpp"

#include <ituGL/shader/Shader.h>

#include <random>
#include <fstream>
#include <iostream>

void GPUGameOfLifeSimulation::Initialize(int width, int height) {
    Shader computeShader(Shader::ComputeShader);

    LoadAndCompileShader(
        computeShader,
        "shaders/gameoflife.comp"
    );

    if (!computeProgram.Build(computeShader))
    {
        std::cout << "Failed to build compute shader\n";
    }

    Resize(width, height);
}

void GPUGameOfLifeSimulation::Resize(int width, int height) {
    this->width = width;
    this->height = height;

    std::vector<std::byte> grid(
        width * height
    );

    std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution dist(0, 1);

    for (auto& cell : grid)
    {
        cell = dist(rng)
            ? std::byte{255}
            : std::byte{0};
    }

    currentTexture.Bind();

    currentTexture.SetImage<std::byte>(
        0,
        width,
        height,
        TextureObject::FormatR,
        TextureObject::InternalFormatR8,
        std::span(grid),
        Data::Type::UByte
    );

    currentTexture.SetParameter(
        TextureObject::ParameterEnum::MinFilter,
        GL_NEAREST
    );

    currentTexture.SetParameter(
        TextureObject::ParameterEnum::MagFilter,
        GL_NEAREST
    );

    currentTexture.SetParameter(
        TextureObject::ParameterEnum::WrapS,
        isWrapping
            ? GL_REPEAT
            : GL_CLAMP_TO_EDGE
    );

    currentTexture.SetParameter(
        TextureObject::ParameterEnum::WrapT,
        isWrapping
            ? GL_REPEAT
            : GL_CLAMP_TO_EDGE
    );

    nextTexture.Bind();

    nextTexture.SetImage(
        0,
        width,
        height,
        TextureObject::FormatR,
        TextureObject::InternalFormatR8
    );

    nextTexture.SetParameter(
        TextureObject::ParameterEnum::MinFilter,
        GL_NEAREST
    );

    nextTexture.SetParameter(
        TextureObject::ParameterEnum::MagFilter,
        GL_NEAREST
    );

    nextTexture.SetParameter(
        TextureObject::ParameterEnum::WrapS,
        isWrapping
            ? GL_REPEAT
            : GL_CLAMP_TO_EDGE
    );

    nextTexture.SetParameter(
        TextureObject::ParameterEnum::WrapT,
        isWrapping
            ? GL_REPEAT
            : GL_CLAMP_TO_EDGE
    );

    Texture2DObject::Unbind();
}

void GPUGameOfLifeSimulation::Update() {
    computeProgram.Use();

    std::cout << "test\n";
    glBindImageTexture(
        0,
        currentTexture.GetHandle(),
        0,
        GL_FALSE,
        0,
        GL_READ_ONLY,
        GL_R8
    );


    glBindImageTexture(
        1,
        nextTexture.GetHandle(),
        0,
        GL_FALSE,
        0,
        GL_WRITE_ONLY,
        GL_R8
    );

    glDispatchCompute(
        (width + 7) / 8,
        (height + 7) / 8,
        1
    );

    glMemoryBarrier(
        GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
    );

    std::swap(
        currentTexture,
        nextTexture
    );
}

void GPUGameOfLifeSimulation::SetCell(int x, int y, bool alive) {
    std::byte value = alive ? ALIVE : DEAD;

    currentTexture.Bind();

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        x,
        y,
        1,
        1,
        GL_RED,
        GL_UNSIGNED_BYTE,
        &value
    );

    Texture2DObject::Unbind();
}

bool GPUGameOfLifeSimulation::GetCell(int x, int y) {
    return false;
}

const Texture2DObject&
GPUGameOfLifeSimulation::GetTexture() {
    return currentTexture;
}

void GPUGameOfLifeSimulation::SetWrapping(bool value) {
    isWrapping = value;
    Resize(width, height);
}
