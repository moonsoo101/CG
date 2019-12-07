#version 330

// input from vertex shader
in vec4 epos;
in vec3 norm;
in vec2 tc;

// texture sampler
uniform sampler2D TEX;

// the only output variable
out vec4 fragColor;

void main()
{
	fragColor = texture2D( TEX, tc );
}
