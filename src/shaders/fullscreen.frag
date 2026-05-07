#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D gridTexture;

uniform vec2 gridSize;

const float GRID_LINE_THICKNESS = 0.03;

void main() {
    float state = texture(gridTexture, vUV).r;

    vec3 color = vec3(state);

    vec2 cellUV = fract(vUV * gridSize);

    bool line =
        cellUV.x < GRID_LINE_THICKNESS ||
        cellUV.x > 1.0 - GRID_LINE_THICKNESS ||
        cellUV.y < GRID_LINE_THICKNESS ||
        cellUV.y > 1.0 - GRID_LINE_THICKNESS;

    if (line) {
        color = vec3(0.5);
    }

    FragColor = vec4(color, 1.0);
}
