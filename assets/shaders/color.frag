#version 330 core

in vec3 vColor;
out vec4 fragColor;

uniform bool uUseColorOverride;
uniform vec4 uColorOverride;

void main()
{
    fragColor = uUseColorOverride ? uColorOverride : vec4(vColor, 1.0);
}
