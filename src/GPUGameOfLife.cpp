#include "GPUGameOfLife.hpp"
#include "util.hpp"
#include <ituGL/shader/Shader.h>
#include <random>
#include <fstream>
#include <iostream>

void GPUGameOfLife::Initialize(int width, int height, bool randomGridGeneration) {
    this->width = width;
    this->height = height;

    InitializeShader();

    InitializeTextures(randomGridGeneration);

    SetWrapping(true);
    SetTrailing(true);
}

void GPUGameOfLife::Step() {
    computeProgram.Use();

    // Bind the current texture to image slot 0 for reading
    glBindImageTexture(
        0,
        readTexture->GetHandle(),
        0,
        GL_FALSE,
        0,
        GL_READ_ONLY,
        GL_RG8
    );

    // Bind the new texture to image slot 1 for writing
    glBindImageTexture(
        1,
        writeTexture->GetHandle(),
        0,
        GL_FALSE,
        0,
        GL_WRITE_ONLY,
        GL_RG8
    );

    // + 7 because width/height might not be divisible by 8
    const GLuint groupsX = (width + 7) / 8;
    const GLuint groupsY = (height + 7) / 8;

    // Run the compute shader
    glDispatchCompute(groupsX, groupsY, 1);

    // Wait until all image writes are finished before proceeding
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Swap the current texture with the new texture
    std::swap(readTexture, writeTexture);
}

void GPUGameOfLife::SetCell(int x, int y, bool alive) {
    readTexture->Bind();

    std::byte value = alive ? ALIVE : DEAD;
    std::byte bytePair[2];

    bytePair[0] = value; // R = state
    bytePair[1] = value; // G = trail

    // x, y, 1, 1 means substitute exactly one pixel in the image texture
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        x,
        y,
        1,
        1,
        GL_RG,
        GL_UNSIGNED_BYTE,
        &bytePair
    );
}

bool GPUGameOfLife::GetCell(int x, int y) {
    return false;
}

void GPUGameOfLife::SetWrapping(bool value) {
    isWrapping = value;
    computeProgram.Use();
    computeProgram.SetUniform(
        computeProgram.GetUniformLocation("isWrapping"),
        isWrapping ? 1 : 0
    );
}

Texture2DObject& GPUGameOfLife::GetTexture() {
    return *readTexture;
}

void GPUGameOfLife::SetTrailing(bool value) {
    isTrailing = value;
    computeProgram.Use();
    computeProgram.SetUniform(
        computeProgram.GetUniformLocation("isTrailing"),
        value ? 1 : 0
    );
}

void GPUGameOfLife::InitializeTextures(bool randomGridGeneration) {
    std::vector<std::byte> grid = GenerateGrid(randomGridGeneration);
    for (int i = 0; i < 2; i++) {
        textures[i].Bind();

        textures[i].SetImage<std::byte>(
            0,
            width,
            height,
            TextureObject::FormatRG,
            TextureObject::InternalFormatRG8,
            std::span(grid),
            Data::Type::UByte
        );

        textures[i].SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
        textures[i].SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    }

    readTexture = &textures[0];
    writeTexture = &textures[1];
}

void GPUGameOfLife::InitializeShader() {
    Shader computeShader(Shader::ComputeShader);
    LoadAndCompileShader(computeShader, "shaders/gameoflife.comp");
    if (!computeProgram.Build(computeShader)) {
        std::cout << "Failed to build compute shader\n";
    }
}
