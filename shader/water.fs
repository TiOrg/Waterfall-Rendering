#version 330 core

out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
}fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec4 materAmbient, materSpecular;
uniform vec4 lightDiffuse, lightAmbient, lightSpecular;
uniform vec4 envirAmbient;

void main()
{
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    
    //vec3 ambient = 0.1 * color;
    
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    //float diff = max(dot(lightDir, normal), 0.0);
    //vec3 diffuse = diff * color;
    
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    vec3 halfwayDir = normalize(lightDir + viewDir);
    //float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    //vec3 specular = vec3(0.2) * spec;
    
    vec4 diffuse, ambient, globalAmt;
    vec4 specular;
    float NdotL, NdotH, NdotR, S, temp, delta;
    float alpha = 0.4;
    
    NdotL = max(dot(normal, lightDir), 0.0);
    NdotH = max(dot(normal, halfwayDir), 0.0);
    NdotR = max(dot(normal, reflectDir), 0.0);
    
    delta = acos(NdotH);
    temp = -1.0 * tan(delta) * tan(delta) / alpha / alpha;
    S = pow(2.71828, temp) / 4.0 / 3.14159 / alpha / alpha / pow(NdotL * NdotR, 0.5);
    
    diffuse = texture(diffuseMap, fs_in.TexCoords) * lightDiffuse;
    globalAmt = envirAmbient * materAmbient;
    ambient = envirAmbient * lightAmbient;
    specular = materSpecular * lightSpecular;
    
    FragColor = NdotL * (diffuse + specular * S) + globalAmt;
}
