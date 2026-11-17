#version 330

// Input vertex attributes from raylib
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Built-in raylib uniform matrix
uniform mat4 mvp;

// Output to fragment shader
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    fragPosition = vertexPosition;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vertexNormal);

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
