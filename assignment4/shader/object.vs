#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

flat out vec3 FragPosFlat;
flat out vec3 NormalFlat;
smooth out vec3 FragPos;
smooth out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

out VS_OUT {
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    
    TexCoords = aTexCoords;
    
    // compute the Tangent, Bitangent, and Normal axes 
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);
    vec3 N = normalize(normalMatrix * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));
    
    // the following light position is the point light's position
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = TBN * vec3(model * vec4(aPos, 0.0));
    
    FragPos = vec3(model * vec4(aPos, 1.0));
    FragPosFlat = vec3(model * vec4(aPos, 1.0));
    Normal = TBN * aNormal;
    NormalFlat = TBN * aNormal;
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
