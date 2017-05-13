#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;
uniform vec4 baseColor = vec4(1,1,1,1);

uniform float stripWidth = 0.2;
uniform float stripDepth = 0.1;
uniform float radius = 1;


in vec3 fposition;
in vec2 ftexCoord;
out vec4 fragColour;

float linearizeDepth(float d){
	float f = 300;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}

float unlinearizeDepth(float z){
	float f = 300;
	float n = 1;
	float d = -((2 * n) / z - f - n) / (f - n);
	return d;
}

void main (void)
{
	float dz = linearizeDepth(gl_FragCoord.z);
	float y = ftexCoord.x * (radius+stripWidth/2);
	float depthOffset = 0;
	if(y<=radius){
		fragColour = vec4(0,0,0,1);
	}
	else{
		fragColour = baseColor;
		depthOffset = (y-radius)/(stripWidth/2) * stripDepth;
	}
	gl_FragDepth = unlinearizeDepth(dz+depthOffset);
}