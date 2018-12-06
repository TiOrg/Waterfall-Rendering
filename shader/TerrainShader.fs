#version 330 core

// Interpolated values from the vertex shaders
//in vec2 UV;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
//uniform sampler2D myTextureSampler;

void main(){

	// Output color = color of the texture at the specified UV
//    color = texture( myTextureSampler, UV ).rgb;
    color = vec4(0.5, 0.7, 0.7, 1.0);
}
