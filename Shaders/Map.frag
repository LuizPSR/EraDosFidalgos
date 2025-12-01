#version 330 core

uniform sampler2D uTexture;
uniform int uTileIndex;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
    vec2 texCoord = (vec2(32.0f, 0.0f) * uTileIndex + vTexCoord * 32.0f) / (vec2(12.0f, 1.0f) * 32.0f);
    FragColor = texture(uTexture, texCoord);
}
