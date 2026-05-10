#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D gridTexture;

void main() {
    float state = texture(gridTexture, vUV).r;

    vec3 color = vec3(state);

    FragColor = vec4(color, 1.0);
}
