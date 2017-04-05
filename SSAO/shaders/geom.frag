#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 normalMat;
uniform float radius;
uniform sampler2D colorTex;


in vec3 fposition;
in vec3 fnormal;
in vec4 fcolor;
//in float finBox;
in vec3 rawPos;

layout (location = 0) out vec4 fragColour;
layout (location = 1) out vec4 gPositionDepth;
layout (location = 2) out vec4 gNormal;

const float NEAR = 1;
const float FAR = 200;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
    //return NEAR + depth * (FAR - NEAR);	
}

void main(void)
{

	gPositionDepth.xyz = fposition;
	gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
	gNormal = vec4(normalize(fnormal),1);
	fragColour = vec4(1,1,1,1);
	//fragColour = vec4(0,0,1,1);
	
	return;

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