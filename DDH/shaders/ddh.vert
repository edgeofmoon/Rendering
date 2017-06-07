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
out vec3 fposition;
out vec3 ftexcoord;

void main (void)
{
	// view aligned
	vec4 refVec = mvMatInv*vec4(0,0,0,1)-vec4(position, 0);
	//vec4 refVec = viewDir;
	vec3 offsetDir = normalize(cross(refVec.xyz, tangent));
	vec3 newPos = position+(texcoord.y-0.5)*stripWidth*offsetDir;
	fposition = (mvMat*vec4(newPos,1)).xyz;
	gl_Position = projMat*vec4(fposition,1);
	//gl_Position = projMat*(mvMat*vec4(position,1));
	
	ftexcoord = texcoord;

	// compute normal
	vec3 view = vec3( 0.0f, 0.0f, 0.0f );
	vec3 c = ( mvMat * vec4( position, 1.0f ) ).xyz - view;
	vec3 t = normalize( mvMat * vec4( tangent, 0.0f ) ).xyz;
	vec3 offset = normalize( cross( view, t ) );
	vec3 newNormal = normalize( cross( offset, t ) );
	newNormal *= sign( dot( newNormal, vec3( 0.0f, 0.0f, 1.0f ) ) );
	fnormal = ( vec4( newNormal, 0.0f ) ).xyz;
}