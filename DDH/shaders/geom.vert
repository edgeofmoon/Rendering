#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform float depthCueing = 1;
uniform vec4 baseColor = vec4(0.5,0.5,0.5,1);

in vec3 position;
in vec3 normal;
in vec4 color;

out vec3 fposition;
out vec3 fnormal;
out vec4 fcolor;
out float finBox;
out vec3 rawPos;

void main (void)
{
	fnormal = normalize((normalMat*vec4(normal,0)).xyz);
	rawPos = position;
	fcolor = color;
	vec4 p = mvMat*vec4(position,1);
	float depth = (-p.z-1) / 300;
	float r = (1.0-depthCueing) + depthCueing*(1.0-(depth-0.48)/0.24);
	vec4 ndcPosition = mvMat*vec4(position+normal*r*radius,1);
	fposition = ndcPosition.xyz/ndcPosition.w;
	gl_Position = projMat*ndcPosition;
}