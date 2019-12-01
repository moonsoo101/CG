#version 330

in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec2 tc;

uniform vec3 up;
uniform vec3 right;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
	vec3 p = right * position.x + up * position.y;
	vec4 wpos = model_matrix * vec4(p, 1);
	vec4 epos = view_matrix * wpos;
	
	gl_Position = projection_matrix * epos;
	tc = texcoord;
}