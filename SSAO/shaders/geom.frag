#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform sampler2D colorTex;
uniform vec4 baseColor = vec4(0.5,0.5,0.5,1);

// lighting
uniform float lightIntensity = 1.0;
uniform float ambient = 0.4;
uniform float diffuse = 0.6;
uniform float specular = 0.0;
uniform float shininess = 32;

// encoding related
uniform float colorInfluence = 1.0;
uniform float valueToSizeInfluence = 0.0;
uniform float valueToTextureInfluence = 0.0;
uniform float valueToTextureRatioInfluence = 0.0;

in vec3 fposition;
in vec3 fnormal;
in vec4 fcolor;
in vec2 ftexCoord;
in float fvalue;

layout (location = 0) out vec4 fragColour;
//layout (location = 1) out vec4 gPositionDepth;
//layout (location = 2) out vec4 gNormal;

const float NEAR = 1;
const float FAR = 300;


float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
    //return NEAR + depth * (FAR - NEAR);	
}

vec3 GetLightedColor(vec3 color){
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
	vec3 rst;
	rst = color*(ambient+diffusion);
	rst += vec3(specule);
	return rst;
}

void main(void)
{

	//gPositionDepth.xyz = fposition;
	//gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
	//gNormal = vec4(normalize(fnormal),0);
	vec4 texColor = vec4(texture2D(colorTex, vec2((fvalue-0.2)/0.8,0.5)).xyz, 1)*valueToTextureInfluence;
	float texRatio = 1 - step(mod(ftexCoord.x*valueToTextureRatioInfluence, 1), fvalue);
	vec4 texRatioColor = vec4(texRatio, texRatio, texRatio, step(1, valueToTextureRatioInfluence));
	vec4 color = fcolor*colorInfluence;
	fragColour = texColor+texRatioColor+color;
	if(colorInfluence+valueToTextureInfluence+valueToTextureRatioInfluence==0){
		fragColour = baseColor;
	}
	fragColour.xyz = GetLightedColor(fragColour.xyz);
	//fragColour.a = baseColor.a;
	//fragColour.xyz = fnormal;
	//fragColour = vec4(texture2D(colorTex, gl_FragCoord.xy/1000).xyz, 1);
}