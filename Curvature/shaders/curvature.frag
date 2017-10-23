#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;

in vec3 ftexcoord;
in vec3 fnormal;
in vec3 ftangent;
in vec3 fpos;
out vec4 fragColour;

vec3 getLighted(vec3 color){
	float gAmbient = 0.2;
	float gDiffuse = 0.6;
	float gSpecular = 0.2;
	float gShininess = 32;

	vec3 lightDir = vec3(0,0,1);
	float ambient = gAmbient;
	float diffusion = gDiffuse*clamp(dot(fnormal,lightDir),0,1);
	diffusion = clamp(diffusion, 0.0, 1.0);     
	vec3 eyeDir = normalize(-fpos);
	vec3 hv = normalize(-reflect(lightDir,fnormal));
	float specular = gSpecular*pow(max(dot(hv,eyeDir),0.0),gShininess);
	specular = clamp(specular, 0.0, 1.0); 
	return color*(ambient+diffusion) + vec3(specular);  
}

float linearizeDepth(float d){
	float f = 300;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}

void main (void)
{
		fragColour = vec4(getLighted(abs(ftangent)),1);
}