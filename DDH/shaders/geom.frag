#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform sampler2D colorTex;
uniform float depthCueing = 1;
uniform vec4 baseColor = vec4(0.5,0.5,0.5,1);

// lighting
uniform float lightIntensity = 1.0;
uniform float ambient = 0.4;
uniform float diffuse = 0.6;
uniform float specular = 0.0;
uniform float shininess = 32;

in vec3 fposition;
in vec3 fnormal;
in vec4 fcolor;
in float finBox;
in vec3 rawPos;

layout (location = 0) out vec4 fragColour;
const float NEAR = 1;
const float FAR = 300;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
    //return NEAR + depth * (FAR - NEAR);	
}

float linearizeDepth2(float d){
	float f = 300;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}

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

void main(void)
{
	fragColour = GetLightedColor(baseColor.xyz);
	//gl_FragDepth = linearizeDepth2(gl_FragCoord.z);
}