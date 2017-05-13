#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;
uniform vec4 baseColor = vec4(1,1,1,1);

uniform float stripWidth = 0.2;
uniform float stripDepth = 0.1;
uniform float radius = 1;

in vec3 position;
in vec2 texCoord;
in float angle;

out vec3 fposition;
out vec2 ftexCoord;

void main (void)
{
	// view aligned
	vec3 refVec = normalize((mvMatInv*vec4(0,0,0,1)-vec4(position, 0)).xyz);
	vec3 upVec = vec3(0, 1, 0);
	vec3 rightVec = cross(upVec, refVec);
	upVec = cross(refVec, rightVec);
	if(length(rightVec)<0.001){
		upVec = vec3(1, 0, 0);
		rightVec = cross(upVec, refVec);
		upVec = cross(refVec, rightVec);
	}
	rightVec = normalize(rightVec);
	upVec = normalize(upVec);
	vec3 offsetDir = cos(angle)*rightVec + sin(angle)*upVec;
	vec3 newPos = position+texCoord.x*(radius+stripWidth/2)*offsetDir;
	vec4 p = mvMat*vec4(newPos,1);
	fposition = p.xyz/p.w;
	gl_Position = projMat*p;
	//gl_Position = projMat*(mvMat*vec4(position,1));

	ftexCoord = texCoord;
}