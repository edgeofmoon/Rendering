#version 330 core

in vec2 TexCoords;

uniform sampler2D gColor;

out vec4 fragColour;

const int blurX = 1;
const int blurY = 1;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(gColor, 0));
    vec4 result = vec4(0.0);
    for (int x = -blurX; x <= blurX; ++x) 
    {
        for (int y = -blurY; y <= blurY; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(gColor, TexCoords + offset);
        }
    }
    fragColour = result / ((2*blurX+1) * (2*blurY+1));
}  