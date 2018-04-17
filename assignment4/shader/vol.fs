#version 330 core
out vec4 FragColor;

in vec3 Pos;

uniform vec4 ourColor; // we set this variable in the OpenGL code.
uniform sampler3D ourTexture;
uniform bool textureMapFlag;

void main()
{
    if (textureMapFlag) {
        vec3 TexCoord = vec3(1.0, 1.0, 1.0) - Pos;
        FragColor = texture(ourTexture, TexCoord);
    }
    else {
        FragColor = ourColor;
    }
    
}
