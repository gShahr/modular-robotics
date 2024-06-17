#version 330 core
in vec4 vertexColor;
in vec2 texCoord;
out vec4 FragColor;
uniform sampler2D tex;
uniform vec2 u_resolution;

void main()
{
	vec3 color = vec3(0.0);
	vec2 st = (texCoord - 0.5) * 2.0;
	// float d = length(st);
	// vec4 f = vec4(1.0, 1.0, 1.0, 1.0);
    FragColor = mix(texture(tex, texCoord), vertexColor, 0.0);
}