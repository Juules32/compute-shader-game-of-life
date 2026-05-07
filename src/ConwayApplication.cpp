#include "ConwayApplication.h"

#include <ituGL/shader/Shader.h>
#include <ituGL/geometry/VertexAttribute.h>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <vector>
#include <imgui.h>

constexpr auto WINDOW_NAME = "Conway's Game of Life";

constexpr int INITIAL_WINDOW_WIDTH = 1280;
constexpr int INITIAL_WINDOW_HEIGHT = 720;

constexpr int INITIAL_GRID_WIDTH = INITIAL_WINDOW_WIDTH / 4;
constexpr int INITIAL_GRID_HEIGHT = INITIAL_WINDOW_HEIGHT / 4;

constexpr int MIN_GRID_SIZE = 32;
constexpr int MAX_GRID_SIZE = 1024;

struct Vertex
{
    glm::vec2 position;
    glm::vec2 uv;
};

ConwayApplication::ConwayApplication() : Application(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, WINDOW_NAME) {}

void ConwayApplication::Initialize() {
    Application::Initialize();

    gridWidth = INITIAL_GRID_WIDTH;
    gridHeight = INITIAL_GRID_HEIGHT;

    imGui.Initialize(GetMainWindow());

    InitializeGeometry();

    InitializeShaders();

    CreateOrUpdateTexture();

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

    if (regenerateGrid) {
        CreateOrUpdateTexture();
        regenerateGrid = false;
    }
}

void ConwayApplication::Render() {
    GetDevice().Clear(Color(0.f, 0.f, 0.f));

    shaderProgram.Use();

    TextureObject::SetActiveTexture(0);
    gridTexture.Bind();

    vao.Bind();

    drawcall.Draw();

    imGui.BeginFrame();

    if (auto window = imGui.UseWindow("Conway Controls"))
    {
        ImGui::SliderInt("Width", &gridWidth, MIN_GRID_SIZE, MAX_GRID_SIZE);
        ImGui::SliderInt("Height", &gridHeight, MIN_GRID_SIZE, MAX_GRID_SIZE);

        if (ImGui::Button("Regenerate"))
        {
            regenerateGrid = true;
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

void ConwayApplication::CreateOrUpdateTexture() {
    std::vector<std::byte> gridData(gridWidth * gridHeight);

    // rand() makes patterns appear, so we use <random>
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1);
    for (auto &cell : gridData) {
        cell = dist(rng) ? std::byte{255} : std::byte{0};
    }

    gridTexture.Bind();

    gridTexture.SetImage<std::byte>(
        0,
        gridWidth,
        gridHeight,
        TextureObject::FormatR,
        TextureObject::InternalFormatR8,
        std::span(gridData),
        Data::Type::UByte
    );

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::MinFilter,
        GL_NEAREST
    );

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::MagFilter,
        GL_NEAREST
    );

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::WrapS,
        GL_CLAMP_TO_EDGE
    );

    gridTexture.SetParameter(
        TextureObject::ParameterEnum::WrapT,
        GL_CLAMP_TO_EDGE
    );

    Texture2DObject::Unbind();
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
    if (!file.is_open())
    {
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
    if (!shader.Compile())
    {
        // Get errors in case of failure
        std::array<char, 2048> errors;
        shader.GetCompilationErrors(errors);
        std::cout << "Error compiling shader: " << path << std::endl;
        std::cout << errors.data() << std::endl;
    }
}
