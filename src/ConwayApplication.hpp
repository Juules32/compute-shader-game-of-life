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
#include "GameOfLife.hpp"
#include "ituGL/utils/DearImGui.h"

const auto WINDOW_NAME = "Conway's Game of Life";

const int INITIAL_WINDOW_WIDTH = 1280;
const int INITIAL_WINDOW_HEIGHT = 720;

const int INITIAL_GRID_WIDTH = INITIAL_WINDOW_WIDTH / 10;
const int INITIAL_GRID_HEIGHT = INITIAL_WINDOW_HEIGHT / 10;
const int MIN_GRID_SIZE = 32;
const int MAX_GRID_SIZE = static_cast<int>(std::pow(2, 13));

const float INITIAL_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 10.0f;
const float MIN_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 2000.0f;
const float MAX_GAME_OF_LIFE_UPDATE_RATE = 1.0f;

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
    void UpdateImplementation();
    void RenderGrid();
    void RenderUI();

    VertexBufferObject vbo;
    VertexArrayObject vao;
    ShaderProgram shaderProgram;
    Drawcall drawcall;
    DearImGui imGui;

    std::unique_ptr<GameOfLife> gameOfLife;

    float currentStepDuration = 0.0f;

    std::unordered_map<int, Window::PressedState> previousKeyStates = {
        {GLFW_KEY_RIGHT, Window::PressedState::Released},
        {GLFW_KEY_ENTER, Window::PressedState::Released},
        {GLFW_KEY_SPACE, Window::PressedState::Released},
        {GLFW_KEY_F1, Window::PressedState::Released},
    };

    // UI State
    int sliderGridWidth = INITIAL_GRID_WIDTH;
    int sliderGridHeight = INITIAL_GRID_HEIGHT;
    float gameStepRate = INITIAL_GAME_OF_LIFE_UPDATE_RATE;
    
    GameOfLifeImplementation selectedGameImplementation = GameOfLifeImplementation::CPU;
    bool isPaused = false;
    bool isUIHidden = false;
    
    std::optional<bool> changeIsWrapping = std::nullopt;
    std::optional<bool> changeIsTrailing = std::nullopt;
    
    bool regenerateGrid = false;
    bool performSingleStep = false;
    bool selectedRandomGridGeneration = false;

    int frameIndex = 0;
    std::array<float, FRAME_BUFFER_SIZE> frameRates{};
    int stepFrameIndex = 0;
    std::array<float, FRAME_BUFFER_SIZE> stepFrameTimes{};
    int renderFrameIndex = 0;
    std::array<float, FRAME_BUFFER_SIZE> renderFrameTimes{};
};
