#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in uvec4 aJoints;
layout(location = 5) in vec4 aWeights;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uOutlineWidth;
uniform bool uUseSkinning;
uniform mat4 uJointMatrices[128];

out vec4 vColor;
out vec2 vTexCoord;
out vec3 vWorldPosition;
out vec3 vWorldNormal;

void main()
{
    vColor = aColor;
    vTexCoord = aTexCoord;

    vec3 position = aPosition;
    vec3 normal = aNormal;
    if (uUseSkinning)
    {
        mat4 skinMatrix =
            aWeights.x * uJointMatrices[aJoints.x] +
            aWeights.y * uJointMatrices[aJoints.y] +
            aWeights.z * uJointMatrices[aJoints.z] +
            aWeights.w * uJointMatrices[aJoints.w];
        position = vec3(skinMatrix * vec4(position, 1.0));
        normal = normalize(mat3(skinMatrix) * normal);
    }

    if (uOutlineWidth > 0.0)
    {
        position += normalize(normal) * uOutlineWidth;
    }

    vec4 worldPosition = uModel * vec4(position, 1.0);
    vWorldPosition = worldPosition.xyz;
    vWorldNormal = normalize(mat3(transpose(inverse(uModel))) * normal);

    gl_Position = uProjection * uView * worldPosition;
}
