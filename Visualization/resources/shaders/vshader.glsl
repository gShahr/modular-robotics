#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 texCoord;

uniform mat4 transform;
uniform mat4 modelmat;
uniform mat4 viewmat;
uniform mat4 projmat;

void main()
{
    gl_Position = projmat * viewmat * modelmat * transform * vec4(aPos, 1.0);
    vertexColor = vec4(aCol, 1.0);
	texCoord = aTexCoord;
}