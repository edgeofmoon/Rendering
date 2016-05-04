#version 330 core

in vec2 TexCoords;

const int kernelSize = 64;

uniform sampler2D gColor;
uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform vec3 samples[kernelSize];
uniform mat4 projection;
uniform float radius;
uniform float occlusionPower;

out vec4 fragColour;

void main()
{
    vec4 fragColor = texture(gColor, TexCoords);
	if(fragColor.a == 0){
		discard;
	}
    vec2 canvasSize = vec2(textureSize(gPositionDepth, 0));
	vec2 noiseScale = vec2(canvasSize.x/4.0, canvasSize.y/4);
    vec3 fragPos = texture(gPositionDepth, TexCoords).xyz;
	vec3 normal = texture(gNormal, TexCoords).rgb;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		vec3 sample = TBN * samples[i]; // From tangent to view-space
		sample = fragPos + sample * radius; 
    
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0 

		float sampleDepth = -texture(gPositionDepth, offset.xy).w; 
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		//rangeCheck = 1;
		occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;   
	} 
	occlusion = 1.0 - (occlusion / kernelSize);
	fragColour.xyz = vec3(pow(occlusion,occlusionPower));
	fragColour.a = 1;
}