#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;
uniform vec4 baseColor = vec4(0.5, 0.5, 0.5, 1);

in vec3 position;
in vec3 normal;

out vec3 fnormal;
out vec3 fposition;

void main (void)
{
	fnormal = normalize((transpose(mvMatInv)*vec4(normal,0)).xyz);
	fposition = (mvMat*vec4(position,1)).xyz;
	gl_Position = projMat*vec4(fposition,1);
	//gl_Position = vec4(position/200,1);
}