#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "camera.h"
#include "balls.h"
#include <vector>

#define LONGITUDE	72
#define LATITUDE	36

//*************************************
// include stb_image with the implementation preprocessor definition
#include "stb_image.h"

//*******************************************************************
extern struct camera cam;
extern ivec2 window_size;
extern ball_t ball;
//*******************************************************************
GLuint  program_balls = 0;
GLuint	ballVAO = 0;
GLuint	vertex_buffer_ball = 0;
GLuint  index_buffer_ball = 0;
GLuint  ball_texture = 0;
//*******************************************************************
std::vector<vertex>             vertex_list;
std::vector<unsigned int>       index_list;
//*******************************************************************
static const char* vert_balls_path = "../bin/shaders/balls.vert";
static const char* frag_balls_path = "../bin/shaders/balls.frag";
static const char* ball_image_path = "../bin/images/ball.jpg";
//*******************************************************************

void init_vertex_list()
{
	vertex_list.clear();

	vertex_list.push_back({ vec3(0.0f,0.0f,1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f,1.0f) });
	for (unsigned int k = 1; k < LATITUDE; k++)
	{
		float theta = PI / float(LATITUDE) * float(k);
		float c = cos(theta), s = sin(theta);
		float z = c;

		for (unsigned int i = 0; i < LONGITUDE; i++)
		{
			float phi = 2 * PI / float(LONGITUDE) * float(i);
			float c2 = cos(phi), s2 = sin(phi);
			float x = s * c2, y = s * s2;
			float coord_x = phi / (2 * PI);
			float coord_y = 1 - (theta / PI);
			vertex_list.push_back({ vec3(x, y, z), vec3(x, y, z), vec2(coord_x, coord_y) });
		}
	}
	vertex_list.push_back({ vec3(0.0f,0.0f,-1.0f), vec3(0.0f, 0.0f, -1.0f), vec2(1.0f, 0.0f) });
}

void init_index_list()
{
	index_list.clear();

	for (uint k = 1; k <= LONGITUDE; k++)
	{
		index_list.push_back(0);	// the origin
		index_list.push_back(k);
		if (k >= LONGITUDE)
			index_list.push_back((k + 1) % LONGITUDE);
		else
			index_list.push_back(k + 1);
	}

	for (uint k = 1; k <= LONGITUDE * (LATITUDE - 2); k++)
	{
		index_list.push_back(k);	// the origin
		index_list.push_back(k + LONGITUDE);
		if (k % LONGITUDE == 0)
			index_list.push_back(k + 1);
		else
			index_list.push_back(k + LONGITUDE + 1);

		index_list.push_back(k);
		if (k % LONGITUDE == 0)
			index_list.push_back(k + 1);
		else
			index_list.push_back(k + LONGITUDE + 1);

		if (k % LONGITUDE == 0)
			index_list.push_back(k - LONGITUDE + 1);
		else
			index_list.push_back(k + 1);
	}

	for (uint k = LONGITUDE * (LATITUDE - 2) + 1; k <= LONGITUDE * (LATITUDE - 1); k++)
	{
		index_list.push_back(k);	// the origin
		index_list.push_back(LONGITUDE * (LATITUDE - 1) + 1);
		if (k >= LONGITUDE * (LATITUDE - 1))
			index_list.push_back(LONGITUDE * (LATITUDE - 2) + 1);
		else
			index_list.push_back(k + 1);
	}
}

void ball_init()
{
	if (!(program_balls = cg_create_program(vert_balls_path, frag_balls_path))) { glfwTerminate(); return; }

	init_vertex_list();
	init_index_list();
	ball.center = vec3(0, -0.5f, 0);
	ball.out = false;

	glGenVertexArrays(1, &ballVAO);
	glGenBuffers(1, &vertex_buffer_ball);
	glGenBuffers(1, &index_buffer_ball);

	glBindVertexArray(ballVAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_ball);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_ball);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * index_list.size(), &index_list[0], GL_STATIC_DRAW);

	cg_bind_vertex_attributes(program_balls);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ball_texture_init()
{
	// load and flip an image
	int width, height, comp = 3;
	unsigned char* pimage0 = stbi_load(ball_image_path, &width, &height, &comp, 3);
	int stride0 = width * comp, stride1 = (stride0 + 3) & (~3);	// 4-byte aligned stride
	unsigned char* pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0); // vertical flip

	// create textures
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &ball_texture);
	glBindTexture(GL_TEXTURE_2D, ball_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8 /* GL_RGB for legacy GL */, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage);

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

	// release the new image
	free(pimage);
}


void render_balls()
{
	glUseProgram(program_balls);
	glBindVertexArray(ballVAO);

	GLint uloc;
	uloc = glGetUniformLocation(program_balls, "model_matrix");       if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, ball.model_matrix);
	uloc = glGetUniformLocation(program_balls, "view_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program_balls, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ball_texture);
	glUniform1i(glGetUniformLocation(program_balls, "TEX"), 0);

	glDrawElements(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr);

}