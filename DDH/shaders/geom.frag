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
layout (location = 1) out vec4 gPositionDepth;
layout (location = 2) out vec4 gNormal;

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

	gPositionDepth.xyz = fposition;
	gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
	gNormal = vec4(normalize(fnormal),1);
	fragColour = vec4(1,0,1,1);
	fragColour.xyz = GetLightedColor(vec3(1,1,1));

	gl_FragDepth = linearizeDepth2(gl_FragCoord.z);

	return;
	// halo code
	float cosTheta = dot(normalize(fnormal), normalize(fposition));
	float intn = pow(2, -8*cosTheta*cosTheta);
	fragColour.rgb *= 1-intn;

	//fragColour = vec4(texture(colorTex, (fposition.xy)/6).rgb, 0.5);

	/*
	// strip pattern
	float freq = 1;
	if(mod(rawPos.x, 1/freq)<0.2){
		fragColour = vec4(0, 0, 0, 1);
	}
	else if(mod(rawPos.y, 1/freq)<0.2){
		fragColour = vec4(0, 0, 0, 1);
	}
	else if(mod(rawPos.z, 1/freq)<0.2){
		fragColour = vec4(0, 0, 0, 1);
	}
	else{
		fragColour = vec4(1, 1, 1, 1);
	}
	*/

	/*
	// code for tube
	if(radius>0.5){
		discard;
	}
	fragColour = vec4(texture(colorTex, (0.8-fcolor.b)/0.8).rgb, 1);
	fragColour = vec4(0.5, 0.5, 0.5, 0.5);
	*/

	/*
	// code for line-ring in box
	if(finBox > 0.5) discard;
	if(radius < 0.5){
		discard;
	}
	fragColour = vec4(0.5, 0.5, 0.5, 0.5);
	*/

	/*
	// code for tube/line mix
	if(finBox>0.5 && radius>0.5){
		discard;
	}
	if(finBox<0.5 && radius<0.5){
		discard;
	}
	gPositionDepth.xyz = fposition;
	gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
	gNormal = vec4(normalize(fnormal),1);
	if(radius>0.5){
		fragColour = vec4(0.5, 0.5, 0.5, 0.5);
	}
	else fragColour = vec4(texture(colorTex, (0.8-fcolor.b)/0.8).rgb, 1);
	
	*/

	/*
	// code for pixel line halo
	if(finBox>0.5){
		discard;
	}
	gPositionDepth.xyz = fposition;
	gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
	gNormal = vec4(normalize(fnormal),1);
	// radius = 1, halo
	if(radius<0.5){
		fragColour = vec4(1,1,1,1);
	}
	// radius = 0, line
	else fragColour = vec4(0.5,0.5,0.5, 1);
	*/
}