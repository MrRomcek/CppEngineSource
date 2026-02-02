#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out VS_OUT {
    vec3 position;
    vec3 color;
    vec2 texCoord;
    vec3 normal;
} vs_out;

void main()
{
    vs_out.position = vec3(uModel * vec4(aPosition, 1.0));
    vs_out.color = aColor;
    vs_out.texCoord = aTexCoord;
    vs_out.normal = mat3(transpose(inverse(uModel))) * aNormal;
    
    gl_Position = uProjection * uView * vec4(vs_out.position, 1.0);
}
