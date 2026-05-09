#include "ConwayApplication.hpp"

#include "util.hpp"

#include <ituGL/shader/Shader.h>
#include <ituGL/geometry/VertexAttribute.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <imgui.h>

#include "CPUGameOfLifeSimulation.hpp"
#include "GPUGameOfLifeSimulation.hpp"

constexpr auto WINDOW_NAME = "Conway's Game of Life";

constexpr int INITIAL_WINDOW_WIDTH = 720;
constexpr int INITIAL_WINDOW_HEIGHT = 720;

constexpr int INITIAL_GRID_WIDTH = 32;
constexpr int INITIAL_GRID_HEIGHT = 32;
constexpr int MIN_GRID_SIZE = 32;
constexpr int MAX_GRID_SIZE = 4096;

constexpr float INITIAL_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 10.0f;
constexpr float MIN_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 2000.0f;
constexpr float MAX_GAME_OF_LIFE_UPDATE_RATE = 1.0f;

using Clock = std::chrono::high_resolution_clock;

struct Vertex {
    glm::vec2 position;
    glm::vec2 uv;
};

ConwayApplication::ConwayApplication() :
    Application(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, WINDOW_NAME),
    uiGridWidth(INITIAL_GRID_WIDTH),
    uiGridHeight(INITIAL_GRID_HEIGHT),
    uiGameOfLifeUpdateRate(INITIAL_GAME_OF_LIFE_UPDATE_RATE),
    uiChangeIsWrapping(std::nullopt),
    uiRegenerateGrid(true),
    currentFrameTime(0),
    uiRandomizeGridGeneration(false),
    uiSimulationType(SimulationType::CPU),
    uiPauseSimulation(false),
    uiPerformSingleStep(false),
    uiRandomGridGeneration(false)
{}

void ConwayApplication::Initialize() {
    Application::Initialize();

    imGui.Initialize(GetMainWindow());

    InitializeGeometry();

    InitializeShaders();

    UpdateSimulation();

    shaderProgram.Use();

    shaderProgram.SetUniform(
        shaderProgram.GetUniformLocation("gridTexture"),
        0
    );
}

void ConwayApplication::UpdateSimulation() {
    if (uiSimulationType == SimulationType::CPU) {
        gameOfLife = std::make_unique<CPUGameOfLifeSimulation>();
    } else {
        gameOfLife = std::make_unique<GPUGameOfLifeSimulation>();
    }
    gameOfLife->Initialize(uiGridWidth, uiGridHeight, uiRandomGridGeneration);
}

void ConwayApplication::Cleanup() {
    imGui.Cleanup();

    Application::Cleanup();
}

void ConwayApplication::Update() {
    Application::Update();


    if (uiPerformSingleStep) {
        gameOfLife->Update();
        uiPerformSingleStep = false;
    } else {
        currentFrameTime += GetDeltaTime();
        if (!uiPauseSimulation && currentFrameTime >= uiGameOfLifeUpdateRate) {
            currentFrameTime = remainderf(currentFrameTime, uiGameOfLifeUpdateRate);

            auto updateStart = Clock::now();

            gameOfLife->Update();

            auto updateEnd = Clock::now();

            const float updateDuration = std::chrono::duration<float, std::milli>(updateEnd - updateStart).count();
            updateFrameRates[updateFrameIndex] = 1000.0f / updateDuration;
            updateFrameIndex = (updateFrameIndex + 1) % updateFrameRates.size();
        }
    }

    if (uiRegenerateGrid) {
        UpdateSimulation();
        shaderProgram.Use();
        shaderProgram.SetUniform(
            shaderProgram.GetUniformLocation("gridSize"),
            glm::vec2(uiGridWidth, uiGridHeight)
        );
        uiRegenerateGrid = false;
    }
    if (uiChangeIsWrapping.has_value()) {
        gameOfLife->SetWrapping(uiChangeIsWrapping.value());
        uiChangeIsWrapping.reset();
    }

}

void ConwayApplication::Render() {
    GetDevice().Clear(Color(0.f, 0.f, 0.f));

    shaderProgram.Use();

    gameOfLife->GetTexture().Bind();

    vao.Bind();

    drawcall.Draw();

    imGui.BeginFrame();
    if (auto window = imGui.UseWindow("Game of Life Controls")) {

        ImGui::Text("Grid Generation");
        ImGui::Separator();

        ImGui::SliderInt("Width", &uiGridWidth, MIN_GRID_SIZE, MAX_GRID_SIZE);
        ImGui::SliderInt("Height", &uiGridHeight, MIN_GRID_SIZE, MAX_GRID_SIZE);

        ImGui::RadioButton("Single Threaded Simulation", (int*)&uiSimulationType, CPU);
        ImGui::SameLine();
        ImGui::RadioButton("Compute Shader Simulation", (int*)&uiSimulationType, GPU);

        ImGui::Checkbox("Random Grid Generation", &uiRandomGridGeneration);

        if (ImGui::Button("Regenerate")) {
            uiRegenerateGrid = true;
        }

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text("Simulation Settings");
        ImGui::Separator();

        ImGui::SliderFloat(
            "Update Rate",
            &uiGameOfLifeUpdateRate,
            MIN_GAME_OF_LIFE_UPDATE_RATE,
            MAX_GAME_OF_LIFE_UPDATE_RATE
        );

        bool isWrapping = gameOfLife->GetWrapping();
        if (ImGui::Checkbox("Enable Wrapping", &isWrapping)) {
            uiChangeIsWrapping = isWrapping;
        }

        ImGui::Checkbox("Pause Simulation", &uiPauseSimulation);

        if (uiPauseSimulation && ImGui::Button("Perform Single Step")) {
            uiPerformSingleStep = true;
        }
    }

    if (auto window = imGui.UseWindow("Performance Metrics")) {
        ImGui::PlotLines(
            "Frame Time",
            updateFrameRates.data(),
            updateFrameRates.size(),
            0,
            nullptr,
            0.0f,
            800.0f,
            ImVec2(0, 80)
        );
    }

    imGui.EndFrame();
}

void ConwayApplication::InitializeGeometry() {
    Vertex vertices[] = {
        { {-1.f, -1.f}, {0.f, 0.f} },
        { { 1.f, -1.f}, {1.f, 0.f} },
        { { 1.f,  1.f}, {1.f, 1.f} },

        { {-1.f, -1.f}, {0.f, 0.f} },
        { { 1.f,  1.f}, {1.f, 1.f} },
        { {-1.f,  1.f}, {0.f, 1.f} },
    };

    vao.Bind();

    vbo.Bind();

    vbo.AllocateData(std::span(vertices, 6), BufferObject::Usage::StaticDraw);


    vao.SetAttribute(
        0,
        VertexAttribute(Data::Type::Float, 2),
        offsetof(Vertex, position),
        sizeof(Vertex)
    );

    vao.SetAttribute(
        1,
        VertexAttribute(Data::Type::Float, 2),
        offsetof(Vertex, uv),
        sizeof(Vertex)
    );

    VertexArrayObject::Unbind();
    VertexBufferObject::Unbind();

    drawcall = Drawcall(Drawcall::Primitive::Triangles, 6, 0);
}

// Load, compile and Build shaders
void ConwayApplication::InitializeShaders() {
    Shader vertexShader(Shader::VertexShader);
    LoadAndCompileShader(vertexShader, "shaders/fullscreen.vert");

    Shader fragmentShader(Shader::FragmentShader);
    LoadAndCompileShader(fragmentShader, "shaders/fullscreen.frag");

    if (!shaderProgram.Build(vertexShader, fragmentShader)) {
        std::cout << "Error linking shaders\n";
    }
}
