#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Color;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform bool useTexture;
uniform vec3 objectColor;

void main() {
    if (useTexture) {
        FragColor = texture(texture_diffuse1, TexCoord);
    } else {
        FragColor = vec4(Color * objectColor, 1.0);
    }
}