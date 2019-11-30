#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "camera.h"
#include "balls.h"
#include <vector>

#define LONGITUDE	72
#define LATITUDE	36
//*******************************************************************
extern struct camera cam;
extern ivec2 window_size;
extern ball_t ball;
//*******************************************************************
GLuint  program_balls = 0;
GLuint	ballVAO = 0;
GLuint	vertex_buffer_ball = 0;
GLuint  index_buffer_ball = 0;
//*******************************************************************
std::vector<vertex>             vertex_list;
std::vector<unsigned int>       index_list;
//*******************************************************************
static const char* vert_balls_path = "../bin/shaders/balls.vert";
static const char* frag_balls_path = "../bin/shaders/balls.frag";
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

void render_balls()
{
	glUseProgram(program_balls);
	glBindVertexArray(ballVAO);

	GLint uloc;
	uloc = glGetUniformLocation(program_balls, "model_matrix");       if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, ball.model_matrix);
	uloc = glGetUniformLocation(program_balls, "view_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program_balls, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);

	glDrawElements(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr);

}