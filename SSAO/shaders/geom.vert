#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform sampler2D colorTex;
uniform vec4 baseColor = vec4(0.5,0.5,0.5,1);
uniform int shape = 2;

// encoding related
uniform float colorInfluence = 1.0;
uniform float valueToSizeInfluence = 0.0;
uniform float valueToTextureInfluence = 0.0;
uniform float valueToTextureRatioInfluence = 0.0;

in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 texCoord;
in float value;

out vec3 fposition;
out vec3 fnormal;
out vec4 fcolor;
out vec2 ftexCoord;
out float fvalue;

void main (void)
{
	fcolor = color;
	ftexCoord = texCoord;
	fvalue = value;
	float offset = 0;
	if(shape == 1){
		// line case, normal is tangent
		vec3 view = vec3( 0.0f, 0.0f, 0.0f );
		vec3 c = ( mvMat * vec4( position, 1.0f ) ).xyz - view;
		vec3 t = normalize( mvMat * vec4( normal, 0.0f ) ).xyz;
		vec3 toffset = normalize( cross( view, t ) );
		vec3 newNormal = normalize( cross( toffset, t ) );
		newNormal *= sign( dot( newNormal, vec3( 0.0f, 0.0f, 1.0f ) ) );
		fnormal = ( vec4( newNormal, 0.0f ) ).xyz;
	}
	else{
		fnormal = normalize((normalMat*vec4(normal,0)).xyz);
		offset = value*valueToSizeInfluence;
		if(valueToSizeInfluence==0) offset = radius;
	}
	vec4 ndcPosition = mvMat*vec4(position+normal*offset*step(0, texCoord.x),1);
	fposition = ndcPosition.xyz/ndcPosition.w;
	gl_Position = projMat*ndcPosition;
}