#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D gridTexture;

void main() {
    vec2 data = texture(gridTexture, vUV).rg;

    float state = data.r;
    float trail = data.g;

    vec3 aliveColor = vec3(0.1, 0.6, 1.0);
    vec3 deadColor  = vec3(0.0, 0.0, 0.0);
    vec3 trailColor = vec3(0.6, 0.0, 1.0);

    vec3 color;

    if (state > 0.5) {
        color = aliveColor;
    } else {
        float t = trail * trail;

        vec3 color = mix(
            vec3(0.0, 0.0, 0.0),
            vec3(0.2, 0.0, 0.6),
            t
        );

        color += vec3(0.1, 0.3, 1.0) * pow(t, 3.0);

        FragColor = vec4(color, 1.0);
        return;
    }

    FragColor = vec4(color, 1.0);
}
