#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;
uniform vec4 baseColor = vec4(0.5, 0.5, 0.5, 1);

// lighting
uniform float lightIntensity = 1.0;
uniform float ambient = 0.4;
uniform float diffuse = 0.6;
uniform float specular = 0.0;
uniform float shininess = 32;

in vec3 fnormal;
in vec3 fposition;

out vec4 fragColour;

vec4 GetLightedColor(vec3 color){
	vec3 lightDir = vec3(0,0,1);
	float diffusion = diffuse*clamp(dot(fnormal,lightDir),0,1);
	//float diffusion = diffuse*abs(dot(normal,lightDir));
	diffusion = max(diffusion, 0.0)*lightIntensity;     
	vec3 eyeDir = normalize(-fposition);
	vec3 hv = normalize(-reflect(lightDir,fnormal));
	float specule = specular*pow(max(dot(hv,eyeDir),0.0),shininess);
	//vec3 hv = normalize(eyeDir+lightDir);
	//float specular = specular*pow(max(dot(hv,normal),0.0),shininess);
	specule = max(specule, 0.0)*lightIntensity;
	vec4 rst;
	rst.xyz = color*(ambient+diffusion);
	rst.xyz += vec3(specule);
	rst.a = 1;
	return rst;
}

float linearizeDepth(float d){
	float f = 300;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}

void main (void)
{
	//float dz = linearizeDepth(gl_FragCoord.z);
	fragColour = GetLightedColor(baseColor.xyz);
	//fragColour = vec4(gl_FragCoord.xyz, 1);
	//gl_FragDepth = linearizeDepth(gl_FragCoord.z)+depthOffset*10;
	//gl_FragDepth = dz;
	//fragColour = vec4(0,0,1-ftexcoord.y,1);
}