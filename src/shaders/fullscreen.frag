#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D gridTexture;

uniform vec2 gridSize;

void main() {
    float state = texture(gridTexture, vUV).r;

    vec3 color = vec3(state);

    vec2 cellUV = fract(vUV * gridSize);

    float thickness = 0.03;

    bool line =
        cellUV.x < thickness ||
        cellUV.x > 1.0 - thickness ||
        cellUV.y < thickness ||
        cellUV.y > 1.0 - thickness;

    if (line) {
        color = vec3(0.5);
    }

    FragColor = vec4(color, 1.0);
}
