#version 330 core
out vec4 FragColor;

struct DirLight {
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};

// declare methods
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

// we need two FragPos and Normal: one for flat shading,
// and one for smooth shading.
flat in vec3 FragPosFlat;
flat in vec3 NormalFlat;
smooth in vec3 FragPos;
smooth in vec3 Normal;

uniform vec3 viewPos;
uniform Material material;
uniform PointLight pointLight;
uniform DirLight dirLight;

uniform bool dirLightFlag;
uniform bool pointLightFlag;
uniform bool flatFlag;

// turn on/off texture mapping and normal mapping
uniform bool diffuseMapFlag;
uniform bool normalMapFlag;

in vec2 TexCoords;
uniform sampler2D normalMap;

in VS_OUT {
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

void main()
{
    // select the Flat or Smoothing models
    vec3 FragPosSelect;
    vec3 NormalSelect;
    if (flatFlag)
    {
        FragPosSelect = FragPosFlat;
        NormalSelect = NormalFlat;
    }
    else
    {
        FragPosSelect = FragPos;
        NormalSelect = Normal;
    }
    
    // turn on/off normal mapping
    vec3 viewDir;
    // viewDir calculated with tangent space
    viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 norm;
    if (normalMapFlag)
    {
        // normal read from the normal map
        norm = texture(normalMap, TexCoords).rgb;
        norm = normalize(norm * 2.0 - 1.0);
    }
    else {
        norm = normalize(NormalSelect);
    }
    
    // aggregate lighting colors from directional light and point light
    vec3 result = vec3(0.0, 0.0, 0.0);
    if (dirLightFlag)
    {
        result += CalcDirLight(dirLight, norm, viewDir);
    }
    if (pointLightFlag)
    {
        result += CalcPointLight(pointLight, norm, FragPosSelect, viewDir);
    }
    
    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;
    // diffuse shading
    vec3 ambient;
    vec3 diffuse;
    if (diffuseMapFlag)
    {
        ambient  = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse  = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    }
    else
    {
        ambient = light.ambient;
        diffuse  = light.diffuse * diff;
    }
    // combine results
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir;
    // compute light direction in tangent space
    lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;
    // turn on/off diffuse map
    vec3 ambient;
    vec3 diffuse;
    if (diffuseMapFlag)
    {
        ambient  = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse  = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    }
    else
    {
        ambient = light.ambient;
        diffuse  = light.diffuse * diff;
    }
    return (ambient + diffuse + specular);
}
