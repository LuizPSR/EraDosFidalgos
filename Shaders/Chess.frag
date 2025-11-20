#version 330 core
out vec4 FragColor;

in vec3 vWorldPos;

uniform mat4 uProj;
uniform mat4 uView;
uniform vec2 uMousePos;

const float GridSize = 32.0;

void main()
{
    float x = vWorldPos.x;
    float y = vWorldPos.y;

    float checker = mod(floor(x / GridSize) + floor(y / GridSize), 2.0);

    vec3 colorA = vec3(0.1, 0.1, 0.15);
    vec3 colorB = vec3(0.7, 0.7, 0.7);
    vec3 colorS = vec3(1.0, 0.0, 0.0);

    vec4 mouseWorldPos = inverse(uProj * uView) * vec4(uMousePos, 0.0, 1.0);
    if (ceil(mouseWorldPos.xy / GridSize) == ceil(vWorldPos.xy / GridSize))
    {
        FragColor = vec4(colorS, 1.0);
    } else {
        FragColor = vec4(mix(colorA, colorB, checker), 1.0);
    }
}
