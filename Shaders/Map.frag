#version 330 core

uniform sampler2D uTexture;
uniform int uTileIndex;

uniform mat4 uProj;
uniform mat4 uView;
uniform vec2 uMousePos;

in vec2 vTexCoord;
in vec3 vWorldPos;

out vec4 FragColor;

const float GridSize = 32.0;

void main()
{
    vec2 texCoord = (vec2(32.0f, 0.0f) * uTileIndex + vTexCoord * 32.0f) / (vec2(12.0f, 1.0f) * 32.0f);
    FragColor = texture(uTexture, texCoord);

    vec4 mouseWorldPos = inverse(uProj * uView) * vec4(uMousePos, 0.0, 1.0);
    if (round(mouseWorldPos.xy / GridSize) == round(vWorldPos.xy / GridSize))
    {
        FragColor *= vec4(1.0f, 0.1f, 0.1f, 1.0f);
    }
}
