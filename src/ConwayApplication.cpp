#include "ConwayApplication.hpp"
#include "SingleThreadedLifeSimulation.hpp"
#include "ComputeShaderLifeSimulation.hpp"
#include "LifeSimulation.hpp"
#include "util.hpp"
#include <ituGL/shader/Shader.h>
#include <ituGL/geometry/VertexAttribute.h>
#include <chrono>
#include <iostream>
#include <imgui.h>

using Clock = std::chrono::high_resolution_clock;

struct Vertex {
    ImVec2 position;
    ImVec2 uv;
};

ConwayApplication::ConwayApplication() :
    Application(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, WINDOW_NAME)
{}

void ConwayApplication::Initialize() {
    Application::Initialize();

    imGui.Initialize(GetMainWindow());

    InitializeGeometry();

    InitializeShaders();

    UpdateImplementation(INITIAL_IS_WRAPPING, INITIAL_IS_TRAILING);

    shaderProgram.Use();

    shaderProgram.SetUniform(shaderProgram.GetUniformLocation("gridTexture"), 0);
}

void ConwayApplication::UpdateImplementation(bool isWrapping, bool isTrailing) {
    if (selectedLifeImplementation == LifeImplementation::SingleThreaded) {
        simulation = std::make_unique<SingleThreadedLifeSimulation>();
    } else {
        simulation = std::make_unique<ComputeShaderLifeSimulation>();
    }
    simulation->Initialize(
        sliderGridWidth,
        sliderGridHeight,
        selectedRandomGridGeneration,
        isWrapping,
        isTrailing
    );

    shaderProgram.Use();
}

void ConwayApplication::Cleanup() {
    imGui.Cleanup();
    Application::Cleanup();
}

void ConwayApplication::Update() {
    Application::Update();

    UpdateInput();

    bool updateSimulation = false;
    if (performSingleStep) {
        updateSimulation = true;
        performSingleStep = false;
    } else {
        currentStepDuration += GetDeltaTime();
        if (!isPaused && currentStepDuration >= simulationStepRate) {
            currentStepDuration = remainderf(currentStepDuration, simulationStepRate);
            updateSimulation = true;
        }
    }
    if (updateSimulation) {
        auto updateStart = Clock::now();

        simulation->Step();

        auto updateEnd = Clock::now();

        const float updateDuration = std::chrono::duration<float, std::milli>(updateEnd - updateStart).count();
        stepFrameTimes[stepFrameIndex] = updateDuration;
        stepFrameIndex = (stepFrameIndex + 1) % stepFrameTimes.size();
    }

    if (regenerateGrid) {
        UpdateImplementation(simulation->GetWrapping(), simulation->GetTrailing());
        regenerateGrid = false;
    }
    if (changeIsWrapping.has_value()) {
        simulation->SetWrapping(changeIsWrapping.value());
        changeIsWrapping.reset();
    }
    if (changeIsTrailing.has_value()) {
        simulation->SetTrailing(changeIsTrailing.value());
        changeIsTrailing.reset();
    }
    if (!isPaused) {
        frameRates[frameIndex] = 1.0f / GetDeltaTime();
        frameIndex = (frameIndex + 1) % frameRates.size();
    }
}

void ConwayApplication::Render() {
    auto renderStart = Clock::now();
    RenderGrid();
    auto renderEnd = Clock::now();
    if (!isPaused) {
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

            int gridWidth = simulation->GetWidth();
            int gridHeight = simulation->GetHeight();

            int x = static_cast<int>((mouseX / windowWidth) * gridWidth);
            int y = static_cast<int>((1.0f - mouseY / windowHeight) * gridHeight);

            if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
                simulation->SetCell(x, y, value);
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
            performSingleStep = true;
        }

        bool spacebarPressed = GetMainWindow().IsKeyPressed(GLFW_KEY_SPACE);
        if (spacebarPressed && previousKeyStates[GLFW_KEY_SPACE] == Window::PressedState::Released) {
            isPaused = !isPaused;
        }

        bool f1Pressed = GetMainWindow().IsKeyPressed(GLFW_KEY_F1);
        if (f1Pressed && previousKeyStates[GLFW_KEY_F1] == Window::PressedState::Released) {
            isUIHidden = !isUIHidden;
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
    simulation->GetTexture().Bind();
    vao.Bind();
    drawcall.Draw();
}

void ConwayApplication::RenderUI() {
    if (isUIHidden) {
        return;
    }

    imGui.BeginFrame();
    if (auto window = imGui.UseWindow("Game of Life Settings")) {
        ImGui::Text("Grid Generation");
        ImGui::Separator();

        ImGui::SliderInt("Grid Width", &sliderGridWidth, MIN_GRID_SIZE, MAX_GRID_SIZE);
        ImGui::SliderInt("Grid Height", &sliderGridHeight, MIN_GRID_SIZE, MAX_GRID_SIZE);

        ImGui::RadioButton("Single Threaded Implementation", (int*)&selectedLifeImplementation, SingleThreaded);
        ImGui::SameLine();
        ImGui::RadioButton("Compute Shader Implementation", (int*)&selectedLifeImplementation, ComputeShader);

        ImGui::Checkbox("Random Grid Generation", &selectedRandomGridGeneration);

        if (ImGui::Button("Regenerate")) {
            regenerateGrid = true;
        }

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text("Other Settings");
        ImGui::Separator();

        ImGui::SliderFloat(
            "Step Rate",
            &simulationStepRate,
            MIN_LIFE_STEP_RATE,
            MAX_LIFE_STEP_RATE
        );

        bool isWrapping = simulation->GetWrapping();
        if (ImGui::Checkbox("Enable Wrapping", &isWrapping)) {
            changeIsWrapping = isWrapping;
        }

        ImGui::SameLine();

        bool isTrailing = simulation->GetTrailing();
        if (ImGui::Checkbox("Enable Trailing", &isTrailing)) {
            changeIsTrailing = isTrailing;
        }

        ImGui::Checkbox("Pause Game", &isPaused);

        if (isPaused && ImGui::Button("Perform Single Step")) {
            performSingleStep = true;
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
        ImGui::Text("Step Frame Time (ms)");
        ImGui::PlotLines(
            "",
            stepFrameTimes.data(),
            static_cast<int>(stepFrameTimes.size()),
            0,
            nullptr,
            0.0f,
            *std::max_element(stepFrameTimes.begin(), stepFrameTimes.end()),
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
