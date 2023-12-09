#version 330 core

layout (location = 0) in vec3 in_Position; 
layout (location = 1) in vec4 in_Color;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec2 vTexCoord;

out vec4 out_Color; 
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main(void)
{
	gl_Position = projection * view * transform * vec4(in_Position, 1.0f);
	FragPos = vec3(transform * vec4(in_Position,1.0));
	// Normal = vec3(transform * vec4(in_Normal,0.0));
	Normal = mat3(transpose(inverse(transform)))* in_Normal;
	TexCoord = vTexCoord;
	out_Color = in_Color;
}