#version 330 core

in vec2 TexCoords;

uniform sampler2D gColor;
uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D gSsao;

uniform float gAmbient;
uniform float gDiffuse;
uniform float gSpecular;
uniform float gShininess;
uniform int gUseSsao;

out vec4 fragColour;


void main() {
	vec4 dcolor = texture(gColor, TexCoords);
	if(dcolor.a == 0){
		discard;
	}
	vec3 color = dcolor.rgb;
	vec3 position = texture(gPositionDepth, TexCoords).xyz;
	vec3 normal = texture(gNormal, TexCoords).xyz;
	float ssao = texture(gSsao, TexCoords).r;

	vec3 lightDir = vec3(0,0,1);
	float ambient = gAmbient;
	float diffusion = gDiffuse*clamp(dot(normal,lightDir),0,1);
	diffusion = clamp(diffusion, 0.0, 1.0);     
	vec3 eyeDir = normalize(-position);
	vec3 hv = normalize(-reflect(lightDir,normal));
	float specular = gSpecular*pow(max(dot(hv,eyeDir),0.0),gShininess);
	//vec3 hv = normalize(eyeDir+lightDir);
	//float specular = gSpecular*pow(max(dot(hv,normal),0.0),gShininess);
	specular = clamp(specular, 0.0, 1.0);     
	fragColour.xyz = color*(ambient+diffusion);
	fragColour.xyz += vec3(specular);
	fragColour.a = 1;
	
	if(gUseSsao != 0){
		fragColour.xyz = fragColour.xyz * ssao;
	}

	return;
	// halo code
	float cosTheta = dot(normalize(normal), normalize(position));
	float intn = pow(2, -8*cosTheta*cosTheta);
	fragColour.rgb *= 1-intn;

}  