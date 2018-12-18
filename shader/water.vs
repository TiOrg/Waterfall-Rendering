#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
}vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vs_out.FragPos=vec3(model * vec4(position, 1.0f));
    vs_out.TexCoords=texCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    //vec3 T = normalize(normalMatrix * tangent);
    //vec3 B = normalize(normalMatrix * bitangent);
    //vec3 N = normalize(normalMatrix * normal);
    
    
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(T, N);
    
    
    //mat3 TBN = mat3(T, B, N)
    
    mat3 TBN = transpose(mat3(T, B, N));//着色语言矩阵mat
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    //vs_out.TangentFragPos = TBN * vs_out.FragPos;
    vs_out.TangentFragPos  = TBN * vec3(model * vec4(position, 0.0));
    
}
