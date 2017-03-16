#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;

uniform float stripWidth;
uniform float stripDepth;
uniform float strokeWidth;
uniform float taperLength;
uniform float depthCueing;

in vec3 ftexcoord;
in vec3 fnormal;
out vec3 fpos;

out vec4 fragColour;

bool isTapered(){
	float u = min(ftexcoord.x, ftexcoord.z-ftexcoord.x);
	if(u<taperLength){
		float v = abs(ftexcoord.y-0.5)*stripWidth;
		//float thres = 0.5*(strokeWidth+u/taperLength*(stripWidth-strokeWidth));
		float thres = 0.5*u/taperLength*stripWidth;
		return v>thres;
	}
}

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
	if(isTapered()) discard;
	float side = 2*abs(ftexcoord.y-0.5);
	float depthOffset = 0;
	float dz = linearizeDepth(gl_FragCoord.z);
	//float thres = strokeWidth*0.5* ((1.0-depthCueing) + depthCueing*(1.0-(dz-0.48)/0.24));
	float thres = strokeWidth*0.5* ((1.0-depthCueing) + depthCueing*(1.0-(dz-0.48)/0.24));
	if(side*stripWidth>thres){
		depthOffset = side*stripDepth;
		//fragColour = vec4(1,1,1,1);
		fragColour = vec4(1,1,1,1);
	}
	else {
		//fragColour = vec4(getLighted(vec3(0.5,0.5,0.5)),1);
		fragColour = vec4(0,0,0,1);
	}
	
	//gl_FragDepth = linearizeDepth(gl_FragCoord.z)+depthOffset*10;
	gl_FragDepth = dz+depthOffset;
	//fragColour = vec4(0,0,1-ftexcoord.y,1);
}