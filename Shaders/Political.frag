#version 330 core

uniform sampler2D uTexture;
uniform int uTileIndex;

uniform mat4 uProj;
uniform mat4 uView;
uniform vec2 uMousePos;
uniform vec4 uRealmColor;

in vec2 vTexCoord;
in vec3 vWorldPos;

out vec4 FragColor;

const float GridSize = 32.0;

void main()
{
    FragColor = uRealmColor;
}
