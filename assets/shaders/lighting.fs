#version 330

// Inputs from vertex shader
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Uniforms for directional cartoon lighting & atmospheric depth
// lightDir: Direction pointing toward the primary sun light source
uniform vec3 lightDir;

// lightColor: RGB intensity of directional sunlight
uniform vec4 lightColor;

// ambientColor: Non-directional base fill light ensuring unlit cube faces remain saturated
uniform vec4 ambientColor;

// viewPos: World-space camera eye position, used for depth fog calculation
uniform vec3 viewPos;

// fogColor: Horizon atmospheric color that distant geometries fade into
uniform vec4 fogColor;

// fogDensity: Exponential distance fog falloff rate (0.0 disables fog)
uniform float fogDensity;

out vec4 finalColor;

void main()
{
    // Normalize surface normal and incoming light direction
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);

    // Lambertian diffuse term
    float NdotL = max(dot(N, L), 0.0);

    // Combine ambient and directional diffuse components
    vec3 lightIntensity = ambientColor.rgb + lightColor.rgb * NdotL;

    // Apply lighting to vertex color
    vec3 litColor = fragColor.rgb * lightIntensity;

    // Distance fog blending
    if (fogDensity > 0.0001)
    {
        float dist = length(viewPos - fragPosition);
        float fogFactor = 1.0 - exp(-dist * fogDensity);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        litColor = mix(litColor, fogColor.rgb, fogFactor);
    }

    finalColor = vec4(litColor, fragColor.a);
}
