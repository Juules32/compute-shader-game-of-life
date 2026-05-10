#include "ConwayApplication.hpp"
#include "CPUGameOfLife.hpp"
#include "GPUGameOfLife.hpp"
#include "util.hpp"

#include <ituGL/shader/Shader.h>
#include <ituGL/geometry/VertexAttribute.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <imgui.h>


using Clock = std::chrono::high_resolution_clock;

struct Vertex {
    glm::vec2 position;
    glm::vec2 uv;
};

ConwayApplication::ConwayApplication() :
    Application(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, WINDOW_NAME)
{}

void ConwayApplication::Initialize() {
    Application::Initialize();

    imGui.Initialize(GetMainWindow());

    InitializeGeometry();

    InitializeShaders();

    UpdateImplementation();

    shaderProgram.Use();

    shaderProgram.SetUniform(
        shaderProgram.GetUniformLocation("gridTexture"),
        0
    );
}

void ConwayApplication::UpdateImplementation() {
    if (uiGameOfLifeImplementation == GameOfLifeImplementation::CPU) {
        gameOfLife = std::make_unique<CPUGameOfLife>();
    } else {
        gameOfLife = std::make_unique<GPUGameOfLife>();
    }
    gameOfLife->Initialize(uiGridWidth, uiGridHeight, uiRandomGridGeneration);

    shaderProgram.Use();
    shaderProgram.SetUniform(
        shaderProgram.GetUniformLocation("gridSize"),
        glm::vec2(uiGridWidth, uiGridHeight)
    );
}

void ConwayApplication::Cleanup() {
    imGui.Cleanup();
    Application::Cleanup();
}

void ConwayApplication::Update() {
    Application::Update();

    UpdateInput();

    bool updateGameOfLife = false;
    if (uiPerformSingleStep) {
        updateGameOfLife = true;
        uiPerformSingleStep = false;
    } else {
        currentFrameTime += GetDeltaTime();
        if (!uiPauseImplementation && currentFrameTime >= uiGameOfLifeUpdateRate) {
            currentFrameTime = remainderf(currentFrameTime, uiGameOfLifeUpdateRate);
            updateGameOfLife = true;
        }
    }
    if (updateGameOfLife) {
        auto updateStart = Clock::now();

        gameOfLife->Step();

        auto updateEnd = Clock::now();

        const float updateDuration = std::chrono::duration<float, std::milli>(updateEnd - updateStart).count();
        updateFrameTimes[updateFrameIndex] = updateDuration;
        updateFrameIndex = (updateFrameIndex + 1) % updateFrameTimes.size();
    }

    if (uiRegenerateGrid) {
        UpdateImplementation();
        uiRegenerateGrid = false;
    }
    if (uiChangeIsWrapping.has_value()) {
        gameOfLife->SetWrapping(uiChangeIsWrapping.value());
        uiChangeIsWrapping.reset();
    }

    if (!uiPauseImplementation) {
        frameRates[frameIndex] = 1.0f / GetDeltaTime();
        frameIndex = (frameIndex + 1) % frameRates.size();
    }
}

void ConwayApplication::Render() {
    auto renderStart = Clock::now();
    RenderGrid();
    auto renderEnd = Clock::now();
    if (!uiPauseImplementation) {
        const float renderDuration = std::chrono::duration<float, std::milli>(renderEnd - renderStart).count();
        renderFrameTimes[renderFrameIndex] = renderDuration;
        renderFrameIndex = (renderFrameIndex + 1) % renderFrameTimes.size();
    }

    RenderUI();
}

void ConwayApplication::UpdateInput() {
    if (!ImGui::GetIO().WantCaptureMouse) {
        bool isLeftMouseButtonPressed = GetMainWindow().IsMouseButtonPressed(Window::MouseButton::Left);
        bool isRightMouseButtonPressed = GetMainWindow().IsMouseButtonPressed(Window::MouseButton::Right);

        if (isLeftMouseButtonPressed || isRightMouseButtonPressed) {
            bool value = isLeftMouseButtonPressed;

            int windowWidth, windowHeight;
            GetMainWindow().GetDimensions(windowWidth, windowHeight);

            glm::vec2 mousePosition = GetMainWindow().GetMousePosition();
            float mouseX = mousePosition.x;
            float mouseY = mousePosition.y;

            int gridWidth = gameOfLife->GetWidth();
            int gridHeight = gameOfLife->GetHeight();
            
            int x = static_cast<int>((mouseX / windowWidth) * gridWidth);
            int y = static_cast<int>((1.0f - mouseY / windowHeight) * gridHeight);

            if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
                gameOfLife->SetCell(x, y, value);
            }
        }
    }

    if (!ImGui::GetIO().WantCaptureKeyboard) {
        bool enterPressed = GetMainWindow().IsKeyPressed(GLFW_KEY_ENTER);
        bool rightArrowPressed = GetMainWindow().IsKeyPressed(GLFW_KEY_RIGHT);
        if (
            (enterPressed && previousKeyStates[GLFW_KEY_ENTER] == Window::PressedState::Released) ||
            (rightArrowPressed && previousKeyStates[GLFW_KEY_RIGHT] == Window::PressedState::Released)
        ) {
            uiPerformSingleStep = true;
        }

        bool spacebarPressed = GetMainWindow().IsKeyPressed(GLFW_KEY_SPACE);
        if (spacebarPressed && previousKeyStates[GLFW_KEY_SPACE] == Window::PressedState::Released) {
            uiPauseImplementation = !uiPauseImplementation;
        }
        
        bool f1Pressed = GetMainWindow().IsKeyPressed(GLFW_KEY_F1);
        if (f1Pressed && previousKeyStates[GLFW_KEY_F1] == Window::PressedState::Released) {
            hideUI = !hideUI;
        }
    }

    for (auto& pair : previousKeyStates) {
        pair.second = GetMainWindow().GetKeyState(pair.first);
    }
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

void ConwayApplication::InitializeShaders() {
    Shader vertexShader(Shader::VertexShader);
    LoadAndCompileShader(vertexShader, "shaders/fullscreen.vert");

    Shader fragmentShader(Shader::FragmentShader);
    LoadAndCompileShader(fragmentShader, "shaders/fullscreen.frag");

    if (!shaderProgram.Build(vertexShader, fragmentShader)) {
        std::cout << "Error linking shaders\n";
    }
}

void ConwayApplication::RenderGrid() {
    shaderProgram.Use();
    gameOfLife->GetTexture().Bind();
    vao.Bind();
    drawcall.Draw();
}

void ConwayApplication::RenderUI() {
    if (hideUI) {
        return;
    }

    imGui.BeginFrame();
    if (auto window = imGui.UseWindow("Game of Life Controls")) {

        ImGui::Text("Grid Generation");
        ImGui::Separator();

        ImGui::SliderInt("Width", &uiGridWidth, MIN_GRID_SIZE, MAX_GRID_SIZE);
        ImGui::SliderInt("Height", &uiGridHeight, MIN_GRID_SIZE, MAX_GRID_SIZE);

        ImGui::RadioButton("Single Threaded Implementation", (int*)&uiGameOfLifeImplementation, CPU);
        ImGui::SameLine();
        ImGui::RadioButton("Compute Shader Implementation", (int*)&uiGameOfLifeImplementation, GPU);

        ImGui::Checkbox("Random Grid Generation", &uiRandomGridGeneration);

        if (ImGui::Button("Regenerate")) {
            uiRegenerateGrid = true;
        }

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text("Implementation Settings");
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

        ImGui::Checkbox("Pause Implementation", &uiPauseImplementation);

        if (uiPauseImplementation && ImGui::Button("Perform Single Step")) {
            uiPerformSingleStep = true;
        }
    }

    if (auto window = imGui.UseWindow("Performance Metrics")) {
        ImGui::Spacing();
        ImGui::Text("Frames Per Second");
        ImGui::PlotLines(
            "",
            frameRates.data(),
            static_cast<int>(frameRates.size()),
            0,
            nullptr,
            0.0f,
            std::min(*std::max_element(frameRates.begin(), frameRates.end()), 500.0f),
            ImVec2(0, 160)
        );

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Update Frame Time (ms)");
        ImGui::PlotLines(
            "",
            updateFrameTimes.data(),
            static_cast<int>(updateFrameTimes.size()),
            0,
            nullptr,
            0.0f,
            *std::max_element(updateFrameTimes.begin(), updateFrameTimes.end()),
            ImVec2(0, 80)
        );

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Render Frame Time (ms)");
        ImGui::PlotLines(
            "",
            renderFrameTimes.data(),
            static_cast<int>(renderFrameTimes.size()),
            0,
            nullptr,
            0.0f,
            *std::max_element(renderFrameTimes.begin(), renderFrameTimes.end()),
            ImVec2(0, 80)
        );
    }

    imGui.EndFrame();
}
