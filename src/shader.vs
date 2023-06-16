#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout(location = 2) in float edit;

out vec4 Colour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 ourColor;

void main()
{
    Colour = vec4(1.0f,1.0f - edit  ,1.0f - edit,0.2f);
	gl_Position = projection * view * model * vec4(aPos, 1.0);

}