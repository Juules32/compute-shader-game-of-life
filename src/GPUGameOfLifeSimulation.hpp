#pragma once

#include "GameOfLifeSimulation.hpp"
#include <vector>

#include "ituGL/shader/ShaderProgram.h"

class GPUGameOfLifeSimulation : public GameOfLifeSimulation {
public:
    void Resize(int width, int height) override;
    void Initialize(int width, int height) override;
    void Update() override;
    void SetCell(int x, int y, bool alive) override;
    bool GetCell(int x, int y) override;
    const Texture2DObject& GetTexture() override;
    void SetWrapping(bool value) override;

private:
    void UpdateTexture();

    Texture2DObject textures[2];
    Texture2DObject* readTex = nullptr;
    Texture2DObject* writeTex = nullptr;

    ShaderProgram computeProgram;
    bool flip = false;
};
