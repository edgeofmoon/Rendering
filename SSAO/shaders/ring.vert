#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform vec3 offset0;
uniform vec3 offset1;

in vec3 position;
in vec3 normal;
in vec4 color;
in vec3 orbit;

out vec3 fposition;
out vec3 fnormal;
out vec4 fcolor;
out vec3 forbit;

out float finBox;
out float fa;

float inBox(vec3 pos, vec3 low, vec3 high){
	if(pos.x<low.x || pos.x>high.x) return 0;
	if(pos.y<low.y || pos.y>high.y) return 0;
	if(pos.z<low.z || pos.z>high.z) return 0;
	return 1;
}

void main (void)
{
	vec3 linePos = position-radius*orbit;
	finBox = inBox(linePos, 
		vec3(-19.5985, -54.2138, -17.6974)+offset0,
		vec3(-12.3999, -43.2969, -8.65376)+offset0) +
		inBox(linePos,
		vec3(14.9959, -46.6653, -19.6276)+offset1,
		vec3(22.1945, -35.7483, -10.584)+offset1);
	fnormal = (normalMat*vec4(normal,0)).xyz;
	forbit  = (normalMat*vec4(orbit,0)).xyz;
	//forbit  = normalize(orbit);
	fcolor.rgb = normalize(color.rgb);
	fcolor.a = color.a;
	fa = color.r/fcolor.r;
	vec4 ndcPosition = mvMat*vec4(position+normal*0,1);
	fposition = ndcPosition.xyz/ndcPosition.w;
	gl_Position = projMat*ndcPosition;
}