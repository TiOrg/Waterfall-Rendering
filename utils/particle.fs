#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D ptexture;

void main()
{    
    FragColor = texture(ptexture, TexCoords);
}
