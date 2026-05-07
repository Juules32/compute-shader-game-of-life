#include "ConwayApplication.hpp"

#include <ituGL/shader/Shader.h>
#include <ituGL/geometry/VertexAttribute.h>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <vector>
#include <imgui.h>

#include "CPUGameOfLifeSimulation.hpp"

constexpr auto WINDOW_NAME = "Conway's Game of Life";

constexpr int INITIAL_WINDOW_WIDTH = 720;
constexpr int INITIAL_WINDOW_HEIGHT = 720;

constexpr int INITIAL_GRID_WIDTH = 32;
constexpr int INITIAL_GRID_HEIGHT = 32;
constexpr int MIN_GRID_SIZE = 32;
constexpr int MAX_GRID_SIZE = 1024;

constexpr float INITIAL_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 10.0f;
constexpr float MIN_GAME_OF_LIFE_UPDATE_RATE = 1.0f / 1000.0f;
constexpr float MAX_GAME_OF_LIFE_UPDATE_RATE = 1.0f;

struct Vertex {
    glm::vec2 position;
    glm::vec2 uv;
};

ConwayApplication::ConwayApplication() : Application(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, WINDOW_NAME) {}

void ConwayApplication::Initialize() {
    Application::Initialize();

    uiGridWidth = INITIAL_GRID_WIDTH;
    uiGridHeight = INITIAL_GRID_HEIGHT;
    uiGameOfLifeUpdateRate = INITIAL_GAME_OF_LIFE_UPDATE_RATE;
    uiChangeIsWrapping = std::nullopt;
    uiRegenerateGrid = true;

    currentFrameTime = 0;

    imGui.Initialize(GetMainWindow());

    // Disable ImGui .ini file
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    InitializeGeometry();

    InitializeShaders();

    gameOfLife = std::make_unique<CPUGameOfLifeSimulation>();
    gameOfLife->Initialize(uiGridWidth, uiGridHeight);

    shaderProgram.Use();

    shaderProgram.SetUniform(
        shaderProgram.GetUniformLocation("gridTexture"),
        0
    );
}

void ConwayApplication::Cleanup() {
    imGui.Cleanup();

    Application::Cleanup();
}

void ConwayApplication::Update() {
    Application::Update();

    currentFrameTime += GetDeltaTime();
    if (currentFrameTime >= uiGameOfLifeUpdateRate) {
        currentFrameTime = remainderf(currentFrameTime, uiGameOfLifeUpdateRate);
        gameOfLife->Update();
    }

    if (uiRegenerateGrid) {
        gameOfLife->Resize(uiGridWidth, uiGridHeight);
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

    TextureObject::SetActiveTexture(0);
    gameOfLife->GetTexture().Bind();

    vao.Bind();

    drawcall.Draw();

    imGui.BeginFrame();

    if (auto window = imGui.UseWindow("Conway Controls")) {
        ImGui::SliderInt("Width", &uiGridWidth, MIN_GRID_SIZE, MAX_GRID_SIZE);
        ImGui::SliderInt("Height", &uiGridHeight, MIN_GRID_SIZE, MAX_GRID_SIZE);
        ImGui::SliderFloat(
            "Update Rate",
            &uiGameOfLifeUpdateRate,
            MIN_GAME_OF_LIFE_UPDATE_RATE,
            MAX_GAME_OF_LIFE_UPDATE_RATE
        );
        if (ImGui::Button("Enable Wrapping")) {
            uiChangeIsWrapping = true;
        }
        if (ImGui::Button("Disable Wrapping")) {
            uiChangeIsWrapping = false;
        }
        if (ImGui::Button("Regenerate")) {
            uiRegenerateGrid = true;
        }
    }

    imGui.EndFrame();

    Application::Render();
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

    constexpr GLsizei stride = sizeof(Vertex);

    vao.SetAttribute(
        0,
        VertexAttribute(Data::Type::Float, 2),
        offsetof(Vertex, position),
        stride
    );

    vao.SetAttribute(
        1,
        VertexAttribute(Data::Type::Float, 2),
        offsetof(Vertex, uv),
        stride
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

void ConwayApplication::LoadAndCompileShader(Shader& shader, const char* path) {
    // Open the file for reading
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Can't find file: " << path << std::endl;
        std::cout << "Is your working directory properly set?" << std::endl;
        return;
    }

    // Dump the contents into a string
    std::stringstream stringStream;
    stringStream << file.rdbuf() << '\0';

    // Set the source code from the string
    shader.SetSource(stringStream.str().c_str());

    // Try to compile
    if (!shader.Compile()) {
        // Get errors in case of failure
        std::array<char, 2048> errors;
        shader.GetCompilationErrors(errors);
        std::cout << "Error compiling shader: " << path << std::endl;
        std::cout << errors.data() << std::endl;
    }
}
