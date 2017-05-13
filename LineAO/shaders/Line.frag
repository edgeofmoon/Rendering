#version 330
uniform vec4 baseColor = vec4(0.5,0.5,0.5,1);

// lighting
uniform float lightIntensity = 1.0;
uniform float ambient = 0.4;
uniform float diffuse = 0.6;
uniform float specular = 0.0;
uniform float shininess = 32;

layout( location = 0 ) out vec4 gColor;
layout( location = 1 ) out vec4 gNDMap;
layout( location = 2 ) out vec3 gZoomMap;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in float Zoom;

vec4 GetLightedColor(vec3 color){
	vec3 lightDir = vec3(0,0,1);
	float diffusion = diffuse*clamp(dot(Normal,lightDir),0,1);
	//float diffusion = diffuse*abs(dot(normal,lightDir));
	diffusion = max(diffusion, 0.0)*lightIntensity;     
	vec3 eyeDir = normalize(-FragPos);
	vec3 hv = normalize(-reflect(lightDir,Normal));
	float specule = specular*pow(max(dot(hv,eyeDir),0.0),shininess);
	//vec3 hv = normalize(eyeDir+lightDir);
	//float specular = specular*pow(max(dot(hv,normal),0.0),shininess);
	specule = max(specule, 0.0)*lightIntensity;
	vec4 rst;
	rst.xyz = color*(ambient+diffusion);
	rst.xyz += vec3(specule);
	rst.a = 1;
	return rst;
}

void main() {

	//float depth = gl_FragCoord.z * gl_FragCoord.w;
	float depth = gl_FragCoord.z;

	gNDMap = vec4( normalize( Normal ), depth );
	gZoomMap = vec3( Zoom, Zoom, Zoom );

	gColor = GetLightedColor(Color);
}