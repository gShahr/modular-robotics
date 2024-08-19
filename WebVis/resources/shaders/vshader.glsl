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
uniform vec3 uColor;
uniform float uTime;

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
    //worldPos.y = worldPos.y - 1.0;
	//worldPos.xyz = worldPos.xyz - 0.5;
	//worldPos.xyz = worldPos.xyz - 20.0;
	
	//worldPos.x = worldPos.x * (1.0 + sin(uTime * 3.0 + worldPos.x) / 45.0);
	//worldPos.y = worldPos.y * (1.0 + cos(uTime * 1.4 + worldPos.y * 2.0) / 45.0);
	//worldPos.z = worldPos.z * (1.0 + sin(uTime * 1.1 + worldPos.z * 3.0) / 45.0);
    
	gl_Position = projmat * viewmat * worldPos;
	//gl_Position.x = pow(abs(gl_Position.x), 0.8) * sign(gl_Position.x);
	//gl_Position.y = pow(abs(gl_Position.y), 0.8) * sign(gl_Position.y);
	
	// gl_Position.x = gl_Position.x * (sin(uTime * 3.0 + gl_Position.x) / 70.0 + 1.0);
	// gl_Position.y = gl_Position.y * (cos(uTime * 1.4 + gl_Position.y * 2.0) / 70.0 + 1.0);
	//gl_Position.w = 1. / gl_Position.w * (sin(timeSec / 4.) + 1.) / 2.;
    vertexColor = vec4(1.0, 1.0, 1.0, 1.0);
	texCoord = aTexCoord;
	surfaceNorm = (ExtractRotationMatrix(modelmat) * ExtractRotationMatrix(transform) * vec4(baseSurfaceNorm, 1.0)).xyz;
}