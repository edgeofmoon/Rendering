#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;

in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 texCoord;

out vec3 fposition;
out vec3 fnormal;
out vec4 fcolor;
//out float finBox;
out vec3 rawPos;
out vec2 ftexCoord;

void main (void)
{
	/*
	finBox = inBox(position, 
		vec3(-19.5985, -54.2138, -17.6974)+offset0,
		vec3(-12.3999, -43.2969, -8.65376)+offset0) +
		inBox(position,
		vec3(14.9959, -46.6653, -19.6276)+offset1,
		vec3(22.1945, -35.7483, -10.584)+offset1);
		*/
	fnormal = (normalMat*vec4(normal,0)).xyz;
	rawPos = position;
	fcolor = color;
	ftexCoord = texCoord;
	vec4 ndcPosition = mvMat*vec4(position+normal*radius,1);
	fposition = ndcPosition.xyz/ndcPosition.w;
	gl_Position = projMat*ndcPosition;
}