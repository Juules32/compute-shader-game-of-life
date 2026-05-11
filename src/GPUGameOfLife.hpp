#pragma once

#include "GameOfLife.hpp"
#include <vector>
#include "ituGL/shader/ShaderProgram.h"

class GPUGameOfLife : public GameOfLife {
public:
    void Initialize(int width, int height, bool randomGridGeneration) override;
    void Step() override;
    void SetCell(int x, int y, bool alive) override;
    bool GetCell(int x, int y) override;
    void SetWrapping(bool value) override;
    Texture2DObject& GetTexture() override;
    void SetTrailing(bool value) override;

private:
    void InitializeTextures(bool randomGridGeneration);
    void InitializeShader();

    ShaderProgram computeProgram;

    bool flip = false;
    Texture2DObject textures[2];
    Texture2DObject* readTexture = nullptr;
    Texture2DObject* writeTexture = nullptr;
};
