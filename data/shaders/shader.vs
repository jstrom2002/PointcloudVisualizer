#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{	
	FragPos = aPos;
	Normal = aNormal;
	gl_PointSize = 5.0;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}