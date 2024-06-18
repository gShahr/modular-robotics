#version 330 core
in vec4 vertexColor;
in vec2 texCoord;
out vec4 FragColor;
uniform sampler2D tex;
uniform vec2 u_resolution;
uniform float timeSec;

void main()
{
	vec3 color = vec3(0.0);
	vec2 st = (texCoord - 0.5) * 2.0;
	// float d = length(st);
	// vec4 f = vec4(1.0, 1.0, 1.0, 1.0);
	FragColor = mix(texture(tex, texCoord), vec4(abs(st.xy), step(0.1, dot(st.xy, st.xy)), 1.0), (sin(timeSec) + 1.0) / 2.0);
}