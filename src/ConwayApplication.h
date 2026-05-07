#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/shader/ShaderProgram.h>
#include "ituGL/texture/Texture2DObject.h"
#include <ituGL/geometry/Drawcall.h>
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
        void CreateOrUpdateTexture();
        void LoadAndCompileShader(Shader& shader, const char* path);

        VertexBufferObject vbo;
        VertexArrayObject vao;
        ShaderProgram shaderProgram;
        Texture2DObject gridTexture;
        Drawcall drawcall;
        DearImGui imGui;

        int gridWidth;
        int gridHeight;
        bool regenerateGrid;
};
