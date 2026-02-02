#version 460 core

in VS_OUT {
    vec3 position;
    vec3 color;
    vec2 texCoord;
    vec3 normal;
} fs_in;

uniform sampler2D uTexture0;
uniform vec3 uCameraPos;
uniform vec3 uLightPos;
uniform vec3 uLightColor;

out vec4 FragColor;

void main()
{
    // Базовое освещение
    vec3 norm = normalize(fs_in.normal);
    vec3 lightDir = normalize(uLightPos - fs_in.position);
    vec3 viewDir = normalize(uCameraPos - fs_in.position);
    
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * fs_in.color;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * fs_in.color;
    
    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    vec3 result = (ambient + diffuse + specular) * uLightColor;
    
    FragColor = vec4(result, 1.0);
}
