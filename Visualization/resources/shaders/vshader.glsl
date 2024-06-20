#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec4 vertexColor;
out vec2 texCoord;
out vec4 worldPos;
out vec3 surfaceNorm;

uniform mat4 transform;
uniform mat4 modelmat;
uniform mat4 viewmat;
uniform mat4 projmat;
uniform vec3 baseSurfaceNorm;
uniform float iTime;

mat4 ExtractRotationMatrix(mat4 T) {
	float sx, sy, sz;
	sx = length(T[0]);
	sy = length(T[1]);
	sz = length(T[2]);
	return mat4(T[0] / sz, T[1] / sy, T[2] / sx, vec4(vec3(0.0), 1.0));
}

void main()
{
	mat4 toModel = modelmat * transform;
	worldPos = toModel * vec4(aPos.xyz, 1.0);
    gl_Position = projmat * viewmat * worldPos;
	gl_Position.x = gl_Position.x * (sin(iTime * 3.0 + gl_Position.x) / 70.0 + 1);
	gl_Position.y = gl_Position.y * (cos(iTime * 1.4 + gl_Position.y * 2) / 70.0 + 1.0);
	//gl_Position.w = 1. / gl_Position.w * (sin(timeSec / 4.) + 1.) / 2.;
    vertexColor = vec4(1.0, 1.0, 1.0, 1.0);
	texCoord = aTexCoord;
	surfaceNorm = (ExtractRotationMatrix(modelmat) * vec4(baseSurfaceNorm, 1.0)).xyz;
}