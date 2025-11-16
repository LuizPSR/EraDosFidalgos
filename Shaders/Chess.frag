#version 330 core
out vec4 FragColor;

in vec3 vWorldPos;

const float GridSize = 32.0;

void main()
{
    float x = vWorldPos.x;
    float y = vWorldPos.y;

    float checker = mod(floor(x / GridSize) + floor(y / GridSize), 2.0);

    vec3 colorA = vec3(0.1, 0.1, 0.15);
    vec3 colorB = vec3(0.7, 0.7, 0.7);

    FragColor = vec4(mix(colorA, colorB, checker), 1.0);
}
