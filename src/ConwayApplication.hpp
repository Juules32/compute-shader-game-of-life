#pragma once

#include <memory>
#include <optional>
#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/geometry/Drawcall.h>

#include "GameOfLifeSimulation.hpp"
#include "ituGL/utils/DearImGui.h"

class ConwayApplication : public Application {
public:
    ConwayApplication();

private:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

    void InitializeGeometry();
    void InitializeShaders();
    void UpdateSimulation();

    VertexBufferObject vbo;
    VertexArrayObject vao;
    ShaderProgram shaderProgram;
    Drawcall drawcall;
    DearImGui imGui;

    int uiGridWidth;
    int uiGridHeight;
    float uiGameOfLifeUpdateRate;
    std::optional<bool> uiChangeIsWrapping;
    bool uiRegenerateGrid;
    SimulationType uiSimulationType;
    bool uiRandomizeGridGeneration;
    bool uiPauseSimulation;
    bool uiPerformSingleStep;
    bool uiRandomGridGeneration;

    std::unique_ptr<GameOfLifeSimulation> gameOfLife;

    float currentFrameTime{};
};
