#version 330 core
in vec4 vertexColor;
in vec2 texCoord;
in vec4 worldPos;
in vec3 surfaceNorm;
out vec4 FragColor;
uniform sampler2D tex;
uniform vec2 iResolution;
uniform float iTime;
uniform vec3 baseSurfaceNorm;

#define WAVEGROWTH 1.5
#define WAVESPEED 0.62
#define WIDTH 0.001

#define fragCoord gl_FragCoord
#define fragColor FragColor

mat4 rotation3d(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(
    oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0
  );
}

float random (in float x) {
    return fract(sin(x*33577.));
}

float Between(float low, float high, float val) {
	return step(low, val) - step(high, val);
}
float Rectangle(vec2 orig, vec2 wh, vec2 st) {
	float x = Between(orig.x, orig.x + wh.x, st.x);
	float y = Between(orig.y, orig.y + wh.y, st.y);
	return x*y;
}

float plot(vec2 st, float pct, float width){
  return  mix(smoothstep( pct-width, pct, st.y), 
              1.0 - smoothstep( pct, pct+width, st.y),
			  smoothstep(0.0, 0.3, (texCoord.y)/1.0));
}

float noise (in float x, float seed) {
    float i = floor(x);
    float f = fract(x + seed);

    // Cubic Hermine Curve
    float u = f * f * f * f * (3.0 - 2.0 * f);

    return mix(random(i + seed), random(i + seed + 1.0), u);
}

float Map(float fromLow, float fromHigh, float toLow, float toHigh, float v) {
	return clamp(toLow + (v - fromLow) / (fromHigh - fromLow) * (toHigh - toLow), toLow, toHigh);
}

void main()
{

	vec3 lightSource = vec3(4.0, 4.0, -10.0);
	float lightAngle = Map(-1.0, 1.0, radians(-120.0), radians(120.0), sin(iTime));
	lightSource = (rotation3d(vec3(0.0, 1.0, 0.0), lightAngle) * vec4(lightSource, 1.0)).xyz;
	float incidentLight = max(0.0, dot(normalize(lightSource - worldPos.xyz), surfaceNorm));
	incidentLight /= length(lightSource - worldPos.xyz);
	incidentLight *= 4.0;
	incidentLight += 0.2;
	incidentLight = clamp(incidentLight, 0.0, 1.0);


	float borderWidth = 0.005;
	float borderMask = 1.0 - Rectangle(vec2(borderWidth), vec2(1.0 - 2*borderWidth), texCoord);
	float interiorMask = 1.0 - borderMask;
	//vec3 borderColor = vec3(0.2, 0.7, 1.0);
	//vec3 borderColor = vec3(0.3, 0.8, 0.1);
	vec3 borderColor = vec3(0.0);
	vec3 border = borderMask * borderColor;

	//float worldSeed = fract( round(worldPos.x + 100.) * 5.4321 + round(worldPos.y + 100.) * 0.1234 + round(worldPos.z + 100.) * 0.6251 );

    vec3 interior = vec3(0.0);
    float pct = 0.0;
	float thisWaveContribution;
	vec3 thisWaveColor;
	int numwaves = 6;
	float alpha = (0.8/numwaves);
    for (int i = 0; i < numwaves; i ++) {
		thisWaveColor = vec3(random(float(i)), random(float(i)+.1), random(float(i)+.3));
		thisWaveColor = clamp(thisWaveColor, 0.1, 0.8);
        thisWaveContribution = noise((texCoord.x + texCoord.y) *(3.0 + float(i)*WAVEGROWTH) + iTime*WAVESPEED, float(i));
		thisWaveContribution = alpha * plot(texCoord, thisWaveContribution, WIDTH);
		interior += thisWaveContribution * thisWaveColor * 2.;
    }
	interior *= interiorMask;
	
	//FragColor = vec4(border + interior, 1.0);
	
	// FragColor = mix(texture(tex, texCoord), vec4(abs(st.xy), step(0.1, dot(st.xy, st.xy)), 1.0), (sin(timeSec) + 1.0) / 2.0);

	vec3 color = border * borderColor;
	FragColor = texture(tex, texCoord) * interiorMask + vec4(color, 1.0);
	FragColor = mix(FragColor, vec4(interior, 1.0), 0.35);
    FragColor.xyz *= incidentLight;

	//FragColor = texture(tex, texCoord);
}