#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

out vec3 vWorldPos;

void main()
{
    vec4 world = uModel * vec4(aPos, 0.0, 1.0);
    vWorldPos = world.xyz;

    gl_Position = uProj * uView * world;
}
