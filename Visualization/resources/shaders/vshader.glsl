#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 texCoord;

uniform mat4 transform;
uniform mat4 modelmat;
uniform mat4 viewmat;
uniform mat4 projmat;

void main()
{
    gl_Position = projmat * viewmat * modelmat * transform * vec4(aPos.xyz, 1.0);
	// gl_Position.w = gl_Position.w * (length(gl_Position.xyz) + 1.);
    vertexColor = vec4(1.0, 1.0, 1.0, 1.0);
	texCoord = aTexCoord;
}