#version 330 core
out vec4 FragColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
in vec3 vColor;

void main()
{
	FragColor = vec4(vColor, 1.0f);
};