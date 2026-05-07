#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D gridTexture;

void main()
{
    float state = texture(gridTexture, vUV).r;      // Sample given texture using UV
    vec3 color = vec3(state);                       // Create color from the raw texture data
    FragColor = vec4(color, 1.0);                   // Set color that GL uses by adding alpha component
}
