#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform sampler1D colorTex;

in vec3 fposition;
in vec3 fnormal;
in vec4 fcolor;
in vec3 forbit;
in float finBox;
in float fa;

layout (location = 0) out vec4 fragColour;
layout (location = 1) out vec4 gPositionDepth;
layout (location = 2) out vec4 gNormal;

const float NEAR = 1;
const float FAR = 200;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
    //return NEAR + depth * (FAR - NEAR);	
}

void main(void)
{
	//if(finBox<0.5){
		discard;
	//}
	vec3 eyeDir = normalize(fposition);
	vec3 tagent = (normalMat*vec4(normalize(fcolor.rgb), 0)).xyz;
	vec3 midDir = cross(cross(eyeDir, tagent),tagent);
	float angle = acos(dot(normalize(midDir), normalize(forbit))) / 3.14;
	if(angle > 120.0/180.0) {
		discard;
	};

	gPositionDepth.xyz = fposition;
	gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
	gNormal = vec4(normalize(fnormal),1);
	fragColour = vec4(texture(colorTex,(fa-0.2)/0.8).rgb, 1);
	//fragColour = vec4(forbit.rgb, 1);
}