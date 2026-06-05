#version 330 core

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vWorldPosition;
in vec3 vWorldNormal;
out vec4 fragColor;

uniform bool uUseColorOverride;
uniform vec4 uColorOverride;
uniform bool uUseTexture;
uniform sampler2D uBaseColorTexture;
uniform bool uUseAlphaMask;
uniform float uAlphaCutoff;
uniform bool uUseToonLighting;
uniform vec3 uLightPosition;
uniform vec3 uCameraPosition;
uniform vec3 uToonShadeColor;
uniform vec3 uToonRimColor;
uniform vec3 uToonEmissionColor;
uniform float uToonThreshold;
uniform float uToonSoftness;
uniform float uToonLitStrength;

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

    if (uUseToonLighting)
    {
        vec3 normal = normalize(vWorldNormal);
        vec3 lightDirection = normalize(uLightPosition - vWorldPosition);
        float halfLambert = dot(normal, lightDirection) * 0.5 + 0.5;
        float litArea = smoothstep(uToonThreshold - uToonSoftness, uToonThreshold + uToonSoftness, halfLambert);
        vec3 toonLight = mix(uToonShadeColor, vec3(uToonLitStrength), litArea);
        baseColor.rgb *= toonLight;

        vec3 viewDirection = normalize(uCameraPosition - vWorldPosition);
        float rim = pow(1.0 - max(dot(normal, viewDirection), 0.0), 2.5);
        baseColor.rgb += uToonRimColor * rim;
        baseColor.rgb += uToonEmissionColor;
    }

    fragColor = baseColor;
}
