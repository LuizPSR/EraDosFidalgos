#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

out vec3 vWorldPos;
out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;

    vec4 world = uModel * vec4(aPos, 0.0, 1.0);
    vWorldPos = world.xyz;

    gl_Position = uProj * uView * world;

}