/* About bricks and a bar */
#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "camera.h"
#include "brick.h"
#include "balls.h"
#include "bar.h"
#include "light.h"
#include "material.h"
#include "particle.h"

//*************************************
// include stb_image with the implementation preprocessor definition
#include "stb_image.h"

//*******************************************************************
extern struct camera cam;
extern std::vector<brick_t> bricks;
extern ivec2 window_size;
extern ball_t ball;
extern bar_t bar;
extern light_t light;
//*************************************
// OpenGL objects
GLuint particle_program = 0;	// ID holder for GPU program
GLuint vertex_buffer_particle = 0;
GLuint particle_tex = 0;
//*******************************************************************
static const char* vert_shader_path = "../bin/shaders/particle.vert";
static const char* frag_shader_path = "../bin/shaders/particle.frag";
static const char* image_path = "../bin/images/Snowflake.png";
//*******************************************************************
std::vector<particle_t> particles;

void particle_init()
{
	if (!(particle_program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return; }

	// load and flip an image
	int width, height, comp;
	unsigned char* pimage0 = stbi_load(image_path, &width, &height, &comp, 4);
	int stride0 = width * comp, stride1 = (stride0 + 4) & (~4);	// 4-byte aligned stride
	unsigned char* pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0); // vertical flip
	stbi_image_free(pimage0); // release the original image

	static vertex vertices[] = { {vec3(-1,-1,0),vec3(0,0,1),vec2(0,0)}, {vec3(1,-1,0),vec3(0,0,1),vec2(1,0)}, {vec3(-1,1,0),vec3(0,0,1),vec2(0,1)}, {vec3(1,1,0),vec3(0,0,1),vec2(1,1)} }; // strip ordering [0, 1, 3, 2]

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(2, &vertex_buffer_particle);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_particle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 4, &vertices[0], GL_STATIC_DRAW);

	// create a particle texture
	glGenTextures(0, &particle_tex);
	glBindTexture(GL_TEXTURE_2D, particle_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage);

	// configure texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	// release the new image
	free(pimage);

	// initialize particles
	constexpr int max_particle = 200;
	particles.resize(max_particle);

}

void render_particle()
{
	glUseProgram(particle_program);

	// bind vertex attributes to your shader program
	cg_bind_vertex_attributes(particle_program);

	GLint uloc;
	for (auto& p : particles)
	{
		uloc = glGetUniformLocation(particle_program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, p.color);
		uloc = glGetUniformLocation(particle_program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, p.pos);
		uloc = glGetUniformLocation(particle_program, "scale"); if (uloc > -1) glUniform1f(uloc, p.scale);

		// render quad vertices
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void particle_update()
{
	// setup texture
	glActiveTexture(GL_TEXTURE0);								// select the texture slot to bind
	glBindTexture(GL_TEXTURE_2D, particle_tex);
	glUniform1i(glGetUniformLocation(particle_program, "TEX"), 0);	 // GL_TEXTURE0
	glBindTexture(GL_TEXTURE_2D, 0);

	GLint uloc = glGetUniformLocation(particle_program, "aspect_ratio"); if (uloc > -1) glUniform1f(uloc, 720/480);

	for (auto& p : particles) p.update();
}
