#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uOutlineWidth;

out vec4 vColor;
out vec2 vTexCoord;

void main()
{
    vColor = aColor;
    vTexCoord = aTexCoord;

    vec3 position = aPosition;
    if (uOutlineWidth > 0.0)
    {
        position += normalize(aNormal) * uOutlineWidth;
    }

    gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
}
