#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;

uniform float stripWidth;
uniform float stripDepth;
uniform float strokeWidth;
uniform float taperLength;
uniform float depthCueing;

in vec3 position;
in vec3 texcoord;
in vec3 tangent;

out vec3 fnormal;
out vec3 fpos;
out vec3 ftexcoord;

void main (void)
{
	// view aligned
	vec4 refVec = mvMatInv*vec4(0,0,0,1)-vec4(position, 0);
	//vec4 refVec = viewDir;
	vec3 offsetDir = normalize(cross(refVec.xyz, tangent));
	vec3 newPos = position+(texcoord.y-0.5)*stripWidth*offsetDir;
	fpos = (mvMat*vec4(newPos,1)).xyz;
	gl_Position = projMat*vec4(fpos,1);
	//gl_Position = projMat*(mvMat*vec4(position,1));

	ftexcoord = texcoord;
	fnormal = cross(cross(tangent, refVec.xyz), tangent);
	fnormal = (transpose(mvMatInv)*vec4(fnormal,0)).xyz;
}