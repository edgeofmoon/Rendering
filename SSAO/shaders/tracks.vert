#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;

in vec3 position;
in vec3 normal;
in vec3 color;

out vec3 fposition;
out vec3 fnormal;
out vec3 fcolor;

void main (void)
{
	fnormal = (normalMat*vec4(normal,0)).xyz;
	fcolor = color;
	vec4 ndcPosition = mvMat*vec4(position,1);
	fposition = ndcPosition.xyz/ndcPosition.w;
	gl_Position = projMat*ndcPosition;
}