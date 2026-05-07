#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

void main()
{
    vUV = aUV;                              // No change in UV
    gl_Position = vec4(aPos, 0.0, 1.0);     // No change in position
}
