/* About bricks and a bar */
#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "camera.h"
#include "brick.h"
#include "balls.h"
#include "bar.h"
#include "light.h"
#include "material.h"

//*************************************
// include stb_image with the implementation preprocessor definition
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//*******************************************************************
inline const float random_range(float min, float max) { return min + rand() / (RAND_MAX / (max - min)); }
extern struct camera cam;
extern std::vector<brick_t> bricks;
extern ivec2 window_size;
extern ball_t ball;
extern bar_t bar;
extern light_t light;
//*******************************************************************
GLuint  brick_texture = 0;
GLuint  program_bricks = 0;
GLuint  program_bars = 0;
GLuint	cubeVAO = 0;
GLuint	cubeVBO = 0;
//*******************************************************************
static const char* vert_bars_path = "../bin/shaders/bars.vert";
static const char* frag_bars_path = "../bin/shaders/bars.frag";
static const char* vert_bricks_path = "../bin/shaders/bricks.vert";
static const char* frag_bricks_path = "../bin/shaders/bricks.frag";
static const char* bricks_image_path = "../bin/images/bricks.jpg";
//*******************************************************************
material_t bricks_material;
//*******************************************************************
void cube_init()
{
	if (!(program_bricks = cg_create_program(vert_bricks_path, frag_bricks_path))) { glfwTerminate(); return; }
	if (!(program_bars = cg_create_program(vert_bars_path, frag_bars_path))) { glfwTerminate(); return; }

	vertex corners_cube[8];
	corners_cube[0].pos = vec3(-1.0f, -1.0f, +1.0f);	corners_cube[0].tex = vec2(0.0f, 0.0f);
	corners_cube[1].pos = vec3(+1.0f, -1.0f, +1.0f);	corners_cube[1].tex = vec2(1.0f, 0.0f);
	corners_cube[2].pos = vec3(+1.0f, -1.0f, -1.0f);	corners_cube[2].tex = vec2(0.0f, 0.0f);
	corners_cube[3].pos = vec3(-1.0f, -1.0f, -1.0f);	corners_cube[3].tex = vec2(0.0f, 0.0f);
	corners_cube[4].pos = vec3(-1.0f, +1.0f, +1.0f);	corners_cube[4].tex = vec2(0.0f, 1.0f);
	corners_cube[5].pos = vec3(+1.0f, +1.0f, +1.0f);	corners_cube[5].tex = vec2(1.0f, 1.0f);
	corners_cube[6].pos = vec3(+1.0f, +1.0f, -1.0f);	corners_cube[6].tex = vec2(0.0f, 0.0f);
	corners_cube[7].pos = vec3(-1.0f, +1.0f, -1.0f);	corners_cube[7].tex = vec2(0.0f, 0.0f);

	vertex surface1[6]
		= {
			corners_cube[0], corners_cube[2], corners_cube[1],
			corners_cube[0], corners_cube[3], corners_cube[2]
	};
	for (int i = 0; i < 6; i++)
		surface1[i].norm = vec3(0.0f, -1.0f, 0.0f);

	vertex surface2[6]
		= {
			corners_cube[0], corners_cube[1], corners_cube[5],
			corners_cube[0], corners_cube[5], corners_cube[4]
	};
	for (int i = 0; i < 6; i++)
		surface2[i].norm = vec3(0.0f, 0.0f, 1.0f);

	vertex surface3[6]
		= {
			corners_cube[1], corners_cube[2], corners_cube[6],
			corners_cube[1], corners_cube[6], corners_cube[5]
	};
	for (int i = 0; i < 6; i++)
		surface3[i].norm = vec3(1.0f, 0.0f, 0.0f);

	vertex surface4[6]
		= {
			corners_cube[2], corners_cube[3], corners_cube[7],
			corners_cube[2], corners_cube[7], corners_cube[6]
	};
	for (int i = 0; i < 6; i++)
		surface4[i].norm = vec3(0.0f, 0.0f, -1.0f);

	vertex surface5[6]
		= {
			corners_cube[3], corners_cube[0], corners_cube[4],
			corners_cube[3], corners_cube[4], corners_cube[7]
	};
	for (int i = 0; i < 6; i++)
		surface5[i].norm = vec3(-1.0f, 0.0f, 0.0f);

	vertex surface6[6]
		= {
		corners_cube[4], corners_cube[5], corners_cube[6],
		corners_cube[4], corners_cube[6], corners_cube[7]
	};
	for (int i = 0; i < 6; i++)
		surface6[i].norm = vec3(0.0f, 1.0f, 0.0f);


	vertex vertices_cube[36]
		= {
			surface1[0], surface1[1], surface1[2], surface1[3], surface1[4], surface1[5],
			surface2[0], surface2[1], surface2[2], surface2[3], surface2[4], surface2[5],
			surface3[0], surface3[1], surface3[2], surface3[3], surface3[4], surface3[5],
			surface4[0], surface4[1], surface4[2], surface4[3], surface4[4], surface4[5],
			surface5[0], surface5[1], surface5[2], surface5[3], surface5[4], surface5[5],
			surface6[0], surface6[1], surface6[2], surface6[3], surface6[4], surface6[5]
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_cube), vertices_cube, GL_STATIC_DRAW);

	cg_bind_vertex_attributes(program_bricks);
	cg_bind_vertex_attributes(program_bars);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void bricks_init()
{
	bricks_material.shininess = 10.0f;
	constexpr int y_unit = 10;
	int x_unit[y_unit] = { 0, };

	for (int i = 0; i < sizeof(x_unit) / sizeof(int); i++)
		x_unit[i] = int(random_range(6, 20));

	for (int j = 0; j < y_unit; j++)
	{
		float y_scale = 0.5f / y_unit;
		float y_unit_size = 2.0f * y_scale;
		float y_offset = j * y_unit_size + (y_unit_size / 2);

		for (int i = 0; i < x_unit[j]; i++)
		{
			float x_scale = 1.0f / x_unit[j];
			float x_unit_size = 2.0f * x_scale;
			float x_offset = i * x_unit_size + (x_unit_size / 2);

			brick_t tmp_brick = {
				true,
				x_scale, y_scale,
				x_offset, y_offset,
				vec2(-1 + x_offset, 1 - y_offset)
			};

			bricks.push_back(tmp_brick);
		}
	}

	bar.pos = vec3(0, -0.8f, 0);
}

void brick_texture_init()
{
	// load and flip an image
	int width, height, comp = 3;
	unsigned char* pimage0 = stbi_load(bricks_image_path, &width, &height, &comp, 3); if (comp == 1) comp = 3; /* convert 1-channel to 3-channel image */
	int stride0 = width * comp, stride1 = (stride0 + 3) & (~3);	// 4-byte aligned stride
	unsigned char* pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0); // vertical flip

	// create textures
	glGenTextures(1, &brick_texture);
	glBindTexture(GL_TEXTURE_2D, brick_texture);
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

void render_bricks()
{
	glUseProgram(program_bricks);
	glBindVertexArray(cubeVAO);

	GLint uloc;
	uloc = glGetUniformLocation(program_bricks, "view_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program_bricks, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);

	// setup light properties
	glUniform4fv(glGetUniformLocation(program_bricks, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program_bricks, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program_bricks, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program_bricks, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program_bricks, "Ka"), 1, bricks_material.ambient);
	glUniform4fv(glGetUniformLocation(program_bricks, "Kd"), 1, bricks_material.diffuse);
	glUniform4fv(glGetUniformLocation(program_bricks, "Ks"), 1, bricks_material.specular);
	glUniform1f(glGetUniformLocation(program_bricks, "shininess"), bricks_material.shininess);

	glActiveTexture(GL_TEXTURE0);								
	glBindTexture(GL_TEXTURE_2D, brick_texture);
	glUniform1i(glGetUniformLocation(program_bricks, "TEX"), 0);	 

	for (unsigned int i = 0; i < bricks.size(); i++)
	{
		if (!bricks[i].bShow)
			continue;
		float x_pos = bricks[i].pos.x;
		float y_pos = bricks[i].pos.y;
		float x_scale = bricks[i].x_scale;
		float y_scale = bricks[i].y_scale;
		mat4 model_matrix = mat4::translate(vec3(x_pos, y_pos, 0)) * mat4::scale(vec3(x_scale, y_scale, 8 * ball.radius));
		glUniformMatrix4fv(glGetUniformLocation(program_bricks, "model_matrix"), 1, GL_TRUE, model_matrix);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void render_bars()
{
	glUseProgram(program_bars);
	glBindVertexArray(cubeVAO);

	GLint uloc;
	uloc = glGetUniformLocation(program_bars, "view_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program_bars, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);

	glUniformMatrix4fv(glGetUniformLocation(program_bars, "model_matrix"), 1, GL_TRUE, bar.model_matrix);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}