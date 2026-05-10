#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D gridTexture;

// Trail shaping
const float TRAIL_FADE = 2.0;
const float TRAIL_GAIN = 1.0;

// Color palette
const vec3 COLOR_LOW  = vec3(0.0, 0.0, 0.1);
const vec3 COLOR_MID  = vec3(0.3, 0.0, 0.6);
const vec3 COLOR_HOT  = vec3(0.2, 0.5, 1.0);

// RGB modulation strength
const float RED_WAVE   = 0.1;
const float GREEN_WAVE = 0.1;
const float BLUE_WAVE  = 0.3;

// Glow settings
const float GLOW_POWER = 3.0;
const float GLOW_STRENGTH = 1.0;

// Alive color override
const vec3 ALIVE_COLOR = vec3(0.3, 0.7, 1.0);

void main() {
    vec2 data = texture(gridTexture, vUV).rg;

    float state = data.r;
    float trail = data.g;

    vec3 color;

    if (state > 0.5) {
        FragColor = vec4(ALIVE_COLOR, 1.0);
        return;
    }

    float t = pow(trail * TRAIL_GAIN, TRAIL_FADE);

    // base plasma gradient
    vec3 base = mix(COLOR_LOW, COLOR_MID, t);

    // RGB wave distortion (gives “electric instability” feel)
    base.r += RED_WAVE   * sin(t * 12.0);
    base.g += GREEN_WAVE * sin(t * 8.0);
    base.b += BLUE_WAVE  * t;

    // hot core glow
    base += COLOR_HOT * pow(t, GLOW_POWER) * GLOW_STRENGTH;

    FragColor = vec4(base, 1.0);
}
