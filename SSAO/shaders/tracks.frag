#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;

in vec3 fposition;
in vec3 fnormal;
in vec3 fcolor;

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
	gPositionDepth.xyz = fposition;
	gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
	gNormal = vec4(normalize(fnormal),1);
	fragColour = vec4(fcolor, 1);
}