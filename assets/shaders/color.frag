#version 330 core

in vec4 vColor;
in vec2 vTexCoord;
out vec4 fragColor;

uniform bool uUseColorOverride;
uniform vec4 uColorOverride;
uniform bool uUseTexture;
uniform sampler2D uBaseColorTexture;
uniform bool uUseAlphaMask;
uniform float uAlphaCutoff;

void main()
{
    if (uUseColorOverride)
    {
        fragColor = uColorOverride;
        return;
    }

    vec4 baseColor = vColor;
    if (uUseTexture)
    {
        baseColor *= texture(uBaseColorTexture, vTexCoord);
    }

    if (uUseAlphaMask && baseColor.a < uAlphaCutoff)
    {
        discard;
    }

    fragColor = baseColor;
}
