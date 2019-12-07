#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "camera.h"
#include "brick.h"
#include "balls.h"
#include "bar.h"
#include "light.h"
#include "material.h"
#include "myrandom.h"
#include "particle.h"
#include <iostream>

//*************************************
// include stb_image with the implementation preprocessor definition
#include "stb_image.h"

//*******************************************************************

#define max_particle 500

extern struct camera cam;
extern std::vector<brick_t> bricks;
extern std::vector<particle_t> particles;
extern ivec2 window_size;
extern ball_t ball;
extern bar_t bar;
extern light_t light;

//*******************************************************************
GLuint program_particles = 0;	// ID holder for GPU program
GLuint particleVAO = 0;
GLuint particleVBO = 0;
GLuint particle_texture = 0;

//*******************************************************************
static const char* vert_particle_path = "../bin/shaders/particle.vert";
static const char* frag_particle_path = "../bin/shaders/particle.frag";
static const char* particle_image_path = "../bin/images/Snowflake.png";

//*******************************************************************
void particle_init()
{
	if (!(program_particles = cg_create_program(vert_particle_path, frag_particle_path))) { glfwTerminate(); return; }

	static vertex vertices[] = { {vec3(-1,-1,0),vec3(0,0,1),vec2(0,0)}, {vec3(1,-1,0),vec3(0,0,1),vec2(1,0)}, {vec3(-1,1,0),vec3(0,0,1),vec2(0,1)}, {vec3(1,1,0),vec3(0,0,1),vec2(1,1)} }; // strip ordering [0, 1, 3, 2]

	glGenVertexArrays(1, &particleVAO);
	glGenBuffers(1, &particleVBO);

	glBindVertexArray(particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 4, &vertices[0], GL_STATIC_DRAW);

	cg_bind_vertex_attributes(program_particles);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	particles.resize(max_particle);
}

void particle_texture_init()
{
	// load and flip an image
	int width, height, comp;
	unsigned char* pimage0 = stbi_load(particle_image_path, &width, &height, &comp, 4);
	int stride0 = width * comp, stride1 = (stride0 + 4) & (~4);	// 4-byte aligned stride
	unsigned char* pimage_particle = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage_particle + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0); // vertical flip
	stbi_image_free(pimage0); // release the original image

	// create textures
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &particle_texture);
	glBindTexture(GL_TEXTURE_2D, particle_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8 /* GL_RGB for legacy GL */, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage_particle);

	// allocate and create mipmap
	int mip_levels = miplevels(window_size.x, window_size.y);
	for (int k = 1, w = width >> 1, h = height >> 1; k < mip_levels; k++, w = max(1, w >> 1), h = max(1, h >> 1))
		glTexImage2D(GL_TEXTURE_2D, k, GL_RGB8 /* GL_RGB for legacy GL */, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);

	// configure texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

}

void reset_particles()
{
	for (auto& p : particles)
		p.reset();
}

bool render_particles(vec3& particle_pos)
{
	
	glUseProgram(program_particles);
	glBindVertexArray(particleVAO);

	mat4 instancing_matrix =
	{
		1, 0, 0, particle_pos.x,
		0, 1, 0, particle_pos.y,
		0, 0, 1, particle_pos.z,			//center.z,
		0, 0, 0, 1
	};

	GLint uloc;
	uloc = glGetUniformLocation(program_particles, "view_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program_particles, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
	uloc = glGetUniformLocation(program_particles, "up");					if (uloc > -1) glUniform3fv(uloc, 1, vec3(cam.view_matrix._21, cam.view_matrix._22, cam.view_matrix._23));
	uloc = glGetUniformLocation(program_particles, "right");				if (uloc > -1) glUniform3fv(uloc, 1, vec3(cam.view_matrix._11, cam.view_matrix._12, cam.view_matrix._13));

	// setup texture
	glActiveTexture(GL_TEXTURE0);								// select the texture slot to bind
	glBindTexture(GL_TEXTURE_2D, particle_texture);
	glUniform1i(glGetUniformLocation(program_particles, "TEX"), 0);	 // GL_TEXTURE0

	for (auto& p : particles)
	{
		p.update();
		uloc = glGetUniformLocation(program_particles, "color");				if (uloc > -1) glUniform4fv(uloc, 1, p.color);
		uloc = glGetUniformLocation(program_particles, "model_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, instancing_matrix * p.model_matrix);

		// render quad vertices
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}


	return true;
}