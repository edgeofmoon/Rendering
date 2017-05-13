#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform sampler2D colorTex;


in vec3 fposition;
in vec3 fnormal;
in vec4 fcolor;
in float finBox;
in vec3 rawPos;

layout (location = 0) out vec4 fragColour;
const float NEAR = 1;
const float FAR = 300;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
    //return NEAR + depth * (FAR - NEAR);	
}

float linearizeDepth2(float d){
	float f = 300;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}

vec3 GetLightedColor(vec3 color){
	vec3 lightColor = vec3( 1.0f, 1.0f, 1.0f );
	vec3 lightDir = vec3(0, 0, 1);
	// illumination
	vec3 viewDir = -normalize(fposition);
	vec3 H = normalize( lightDir + viewDir );
	// ambient
	float ambientStrength = 0.1f;
	vec3 ambient = lightColor * ambientStrength;
	// diffuse
	float diffuseStrength = max( dot( lightDir, fnormal ), 0.0f );
	vec3 diffuse = lightColor * diffuseStrength;
	// specular
	float specularStrength = pow( max( dot( H, fnormal ), 0.0f ), 64.0f );
	if( diffuseStrength <= 0.0f ) specularStrength = 0.0f;
	vec3 specular = lightColor * specularStrength;

	vec3 result = ( ambient + diffuse + specular ) * color;

	return result;
}

void main(void)
{
	fragColour.xyz = GetLightedColor(vec3(1,1,1));
	fragColour.w = 1;
	gl_FragDepth = linearizeDepth2(gl_FragCoord.z);
}