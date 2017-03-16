#version 330

in vec3 position;
out vec2 TexCoords;

void main (void)
{
	TexCoords = position.xy;
	gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, position.z, 1);
}