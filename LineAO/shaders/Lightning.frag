#version 330

#define SCALES 4
#define SAMPLES 32

uniform sampler2D gColor;
uniform sampler2D aoColor;
uniform sampler2D gNDMap;
uniform sampler2D gZoomMap;

uniform int blurX = 0;
uniform int blurY = 0;

in vec2 TexCoords;
out vec4 fragColour;

vec4 blur() {
    vec2 texelSize = 1.0 / vec2(textureSize(aoColor, 0));
	vec4 sum = vec4( 0.0f );
    for (int x = -blurX; x <= blurX; ++x) {
        for (int y = -blurY; y <= blurY; ++y){
            vec2 offset = vec2(float(x), float(y)) * texelSize;
			sum += texture(aoColor, TexCoords + offset);
		}
	}

	return sum / ((2*blurX+1) * (2*blurY+1));
}
float linearizeDepth(float d){
	float f = 200;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}
void main() {
	vec4 ao = blur();
	fragColour.xyz = texture( gColor, TexCoords ).rgb * ao.x;
	//fragColour.xyz = ao.xyz;
	//fragColour.xyz = texture( gColor, TexCoords ).rgb;
	fragColour.w = texture( gColor, TexCoords ).a;
}
