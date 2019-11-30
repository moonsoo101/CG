#version 330

// input from vertex shader
in vec4 epos;
in vec3 norm;
in vec2 tc;

// the only output variable
out vec4 fragColor;

void main()
{
	fragColor = vec4(tc.xy, 0, 1);
}
