#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 modelViewMat;
uniform mat4 perspProjMat;
uniform mat3 normalMat;

uniform float time;

out vec2 texture_coord;

out vec3 normalVect;
out vec3 lightVect;
out vec3 eyeVect;
out vec3 halfWayVect;
out vec3 reflectVect;

void main()
{
    gl_Position = perspProjMat * modelViewMat * vec4(position, 1.0);

	float tex_x = (position.x + time/20.0) / 8.0 + 0.5;
	float tex_y = 0.5 - (position.y + time/25.0) / 5.0;
	texture_coord = vec2(tex_x, tex_y);

	vec3 eyePos = vec3(0.0, 0.0, 5.0);
	vec3 lightPos = vec3(1.0, 3.0, 0.0);
	vec3 ptVertex = vec3(modelViewMat * vec4(position, 1.0));

	eyeVect = normalize(eyePos - ptVertex);
	lightVect = normalize(lightPos - ptVertex);
	halfWayVect = eyeVect + lightVect;
	normalVect = normalMat * normal;
	reflectVect = 1.0 * eyeVect - 2.0 * dot(-1.0*eyeVect, normalVect) * normalVect;
}
