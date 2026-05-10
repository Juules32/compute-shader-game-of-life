#pragma once

#include <memory>
#include <optional>
#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/geometry/Drawcall.h>
#include <array>
#include <unordered_map>

#include "GameOfLifeSimulation.hpp"
#include "ituGL/utils/DearImGui.h"

constexpr auto WINDOW_NAME = "Conway's Game of Life";

constexpr int INITIAL_WINDOW_WIDTH = 720;
constexpr int INITIAL_WINDOW_HEIGHT = 720;

constexpr int INITIAL_GRID_WIDTH = 32;
constexpr int INITIAL_GRID_HEIGHT = 32;
constexpr int MIN_GRID_SIZE = 32;
const int MAX_GRID_SIZE = static_cast<int>(std::pow(2, 13));

constexpr float INITIAL_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 10.0f;
constexpr float MIN_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 2000.0f;
constexpr float MAX_GAME_OF_LIFE_UPDATE_RATE = 1.0f;

const int FRAME_BUFFER_SIZE = 2048;

class ConwayApplication : public Application {
public:
    ConwayApplication();

private:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

    void UpdateInput();
    void InitializeGeometry();
    void InitializeShaders();
    void UpdateSimulation();
    void RenderGrid();
    void RenderUI();

    VertexBufferObject vbo;
    VertexArrayObject vao;
    ShaderProgram shaderProgram;
    Drawcall drawcall;
    DearImGui imGui;

    int uiGridWidth = INITIAL_GRID_WIDTH;
    int uiGridHeight = INITIAL_GRID_HEIGHT;
    float uiGameOfLifeUpdateRate = INITIAL_GAME_OF_LIFE_UPDATE_RATE;
    std::optional<bool> uiChangeIsWrapping = std::nullopt;
    bool uiRegenerateGrid = false;
    SimulationType uiSimulationType = SimulationType::CPU;
    bool uiPauseSimulation = false;
    bool uiPerformSingleStep = false;
    bool uiRandomGridGeneration = false;
    bool hideUI = false;
    std::unordered_map<int, Window::PressedState> previousKeyStates = {
        {GLFW_KEY_RIGHT, Window::PressedState::Released},
        {GLFW_KEY_ENTER, Window::PressedState::Released},
        {GLFW_KEY_SPACE, Window::PressedState::Released},
        {GLFW_KEY_F1, Window::PressedState::Released},
    };

    std::unique_ptr<GameOfLifeSimulation> gameOfLife;

    float currentFrameTime = 0.0f;

    int frameIndex = 0;
    std::array<float, FRAME_BUFFER_SIZE> frameRates{};

    int updateFrameIndex = 0;
    std::array<float, FRAME_BUFFER_SIZE> updateFrameTimes{};

    int renderFrameIndex = 0;
    std::array<float, FRAME_BUFFER_SIZE> renderFrameTimes{};
};
