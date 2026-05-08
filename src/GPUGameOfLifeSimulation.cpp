#include "GPUGameOfLifeSimulation.hpp"
#include "util.hpp"

#include <ituGL/shader/Shader.h>

#include <random>
#include <fstream>
#include <iostream>

void GPUGameOfLifeSimulation::Initialize(int width, int height) {
    Shader computeShader(Shader::ComputeShader);

    LoadAndCompileShader(computeShader, "shaders/gameoflife.comp");

    if (!computeProgram.Build(computeShader)) {
        std::cout << "Failed to build compute shader\n";
    }

    computeProgram.Use();
    computeProgram.SetUniform(
        computeProgram.GetUniformLocation("isWrapping"),
        isWrapping ? 1 : 0
    );

    Resize(width, height);

    // pointer setup (start state)
    readTex  = &textures[0];
    writeTex = &textures[1];
}

void GPUGameOfLifeSimulation::Resize(int width, int height) {
    this->width = width;
    this->height = height;

    std::vector<std::byte> grid(width * height);

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1);

    for (auto& cell : grid) {
        cell = dist(rng) ? std::byte{255} : std::byte{0};
    }

    for (int i = 0; i < 2; i++) {
        textures[i].Bind();

        textures[i].SetImage<std::byte>(
            0,
            width,
            height,
            TextureObject::FormatR,
            TextureObject::InternalFormatR8,
            std::span(grid),
            Data::Type::UByte
        );

        textures[i].SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
        textures[i].SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);

        textures[i].SetParameter(
            TextureObject::ParameterEnum::WrapS,
            isWrapping ? GL_REPEAT : GL_CLAMP_TO_EDGE
        );

        textures[i].SetParameter(
            TextureObject::ParameterEnum::WrapT,
            isWrapping ? GL_REPEAT : GL_CLAMP_TO_EDGE
        );
    }

    Texture2DObject::Unbind();
}

void GPUGameOfLifeSimulation::Update() {
    computeProgram.Use();

    glBindImageTexture(
        0,
        readTex->GetHandle(),
        0,
        GL_FALSE,
        0,
        GL_READ_ONLY,
        GL_R8
    );

    glBindImageTexture(
        1,
        writeTex->GetHandle(),
        0,
        GL_FALSE,
        0,
        GL_WRITE_ONLY,
        GL_R8
    );

    GLuint groupsX = (width + 7) / 8;
    GLuint groupsY = (height + 7) / 8;

    glDispatchCompute(groupsX, groupsY, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // swap pointers (THIS is the key change)
    std::swap(readTex, writeTex);
}

void GPUGameOfLifeSimulation::SetCell(int x, int y, bool alive) {
    std::byte value = alive ? ALIVE : DEAD;

    readTex->Bind();

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

const Texture2DObject& GPUGameOfLifeSimulation::GetTexture() {
    return *readTex;
}

void GPUGameOfLifeSimulation::SetWrapping(bool value) {
    isWrapping = value;
    computeProgram.Use();
    computeProgram.SetUniform(
        computeProgram.GetUniformLocation("isWrapping"),
        isWrapping ? 1 : 0
    );
}
