#version 330

uniform mat4x4 mvMat;
uniform mat4x4 projMat;
uniform mat4x4 mvMatInv;

uniform float stripWidth;
uniform float stripDepth;
uniform float strokeWidth;
uniform float taperLength;
uniform float depthCueing;

uniform vec4 baseColor = vec4(0,0,0,1);
// lighting
uniform float lightIntensity = 1.0;
uniform float ambient = 0.4;
uniform float diffuse = 0.6;
uniform float specular = 0.0;
uniform float shininess = 32;


in vec3 ftexcoord;
in vec3 fnormal;
out vec3 fposition;

out vec4 fragColour;

bool isTapered(){
	float u = min(ftexcoord.x, ftexcoord.z-ftexcoord.x);
	if(u<taperLength){
		float v = abs(ftexcoord.y-0.5)*stripWidth;
		//float thres = 0.5*(strokeWidth+u/taperLength*(stripWidth-strokeWidth));
		float thres = 0.5*u/taperLength*stripWidth;
		return v>thres;
	}
	return false;
}

vec3 GetLightedColor(vec3 color){
	vec3 lightDir = vec3(0,0,1);
	float diffusion = diffuse*clamp(dot(fnormal,lightDir),0,1);
	//float diffusion = diffuse*abs(dot(normal,lightDir));
	diffusion = max(diffusion, 0.0)*lightIntensity;     
	vec3 eyeDir = normalize(-fposition);
	vec3 hv = normalize(-reflect(lightDir,fnormal));
	float specule = specular*pow(max(dot(hv,eyeDir),0.0),shininess);
	//vec3 hv = normalize(eyeDir+lightDir);
	//float specular = specular*pow(max(dot(hv,normal),0.0),shininess);
	specule = max(specule, 0.0)*lightIntensity;
	vec3 rst;
	rst = color*(ambient+diffusion);
	rst += vec3(specule);
	return rst;
}


// output range n/f~1
float linearizeDepth(float d){
	float f = 300;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}

// output range -1~1
float unlinearizeDepth(float z){
	float f = 300;
	float n = 1;
	float d = -((2 * n) / z - f - n) / (f - n);
	return d;
}

void main (void)
{
	if(isTapered()) discard;
	float side = abs(ftexcoord.y-0.5);
	float depthOffset = 0;
	float dz = linearizeDepth(gl_FragCoord.z);
	//float thres = strokeWidth*0.5* ((1.0-depthCueing) + depthCueing*(1.0-(dz-0.48)/0.24));
	float thres = strokeWidth*0.5* ((1.0-depthCueing) + depthCueing*(1.0-(dz-0.48)/0.24));
	if(side*stripWidth>thres){
		depthOffset = side*stripDepth;
		//fragColour = vec4(1,1,1,1);
		// set alpha to 0 to mark it as halo
		fragColour = vec4(1,1,1,0);
	}
	else {
		fragColour = vec4(GetLightedColor(baseColor.xyz),1);
		//fragColour.xyz = fnormal;
	}
	
	//gl_FragDepth = linearizeDepth(gl_FragCoord.z)+depthOffset*10;
	gl_FragDepth = unlinearizeDepth(dz+depthOffset);
	//gl_FragDepth = gl_FragCoord.z;
	//gl_FragDepth = (dz+depthOffset);
	//gl_FragDepth = gl_FragCoord.z
	//fragColour = vec4(0,0,1-ftexcoord.y,1);
}