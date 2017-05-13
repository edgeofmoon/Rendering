#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius = 0.4;
uniform int hasNormal = 0;
uniform vec4 baseColor = vec4(0.5,0.5,0.5,1);

in vec3 position;
in vec3 tangent;
in vec3 normal;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out float Zoom;

void main() {
	
	vec3 pos = position;
	if(hasNormal == 1){
		pos += normal * radius;
		Normal = normalize((normalMat*vec4(normal,0)).xyz);
	}
	else{
		vec3 view = vec3( 0.0f, 0.0f, 0.0f );
		vec3 c = ( mvMat * vec4( pos, 1.0f ) ).xyz - view;
		vec3 t = normalize( mvMat * vec4( tangent, 0.0f ) ).xyz;
		vec3 offset = normalize( cross( view, t ) );
		vec3 newNormal = normalize( cross( offset, t ) );
		newNormal *= sign( dot( newNormal, vec3( 0.0f, 0.0f, 1.0f ) ) );
		Normal = ( vec4( newNormal, 0.0f ) ).xyz;
	}
	Zoom = length( ( mvMat * normalize( vec4( 1.0f, 1.0f, 1.0f, 0.0f ) ) ).xyz );
	FragPos = ( mvMat * vec4( pos, 1.0f ) ).xyz;;
	//Color = normalize( abs( tangent ) );
	Color = baseColor.xyz;
	//Color = vec3(0.3);
	gl_Position = projMat * vec4(FragPos, 1);
}