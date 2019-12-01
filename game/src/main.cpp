#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "trackball.h"
#include "balls.h"
#include "bar.h"
#include "brick.h"
#include "camera.h"
#include "light.h"
#include "particle.h"
#include <vector>
#include "dear_imgui/imgui.h"
#include "dear_imgui/imgui_impl_glfw.h"
#include "dear_imgui/imgui_impl_opengl3.h"
#include "irrKlang/irrKlang.h"
#pragma comment(lib, "irrKlang.lib")

void text_init();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color);

extern void ball_init();
extern void render_balls();
extern void cube_init();
extern void bricks_init();
extern void render_bricks();
extern void brick_texture_init();
extern void render_bars();
extern void particle_init();
extern void particle_update();
extern void render_particle();
//*************************************
// global constants
static const char* window_name = "cgbase - texture";
static const char* vert_shader_path = "../bin/shaders/texture.vert";
static const char* frag_shader_path = "../bin/shaders/texture.frag";
static const char* image_path = "../bin/images/bricks.jpg";
static const char* bgm_path = "../bin/bgm.mp3";
static const char* effect_path = "../bin/brick.mp3";

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = ivec2(1024, 1024);	// initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;	// ID holder for GPU program
GLuint	program_floor = 0;

GLuint	floorVAO = 0;
GLuint	vertex_buffer_floor = 0;

//*******************************************************************
// irrKlang objects
irrklang::ISoundEngine* engine;
irrklang::ISoundSource* bgm;
irrklang::ISoundSource* effect;
//*************************************
// global variables
int		frame = 0;				// index of rendering frames
bool	b_wireframe = false;	// this is the default
float	cur_t = 0.0f;
float	pre_t = 0.0f;
bool	b_game_start = false;
bool	b_game_pause = false;
bool	b_die = false;

//*************************************
// scene objects
camera		cam;
ball_t		ball;
bar_t		bar(ball.radius);
trackball	tb;
std::vector<brick_t> bricks;
light_t     light;

//*************************************
void update()
{
	// update projection matrix
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar);
	//particle_update();

	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
}

void render()
{

	// notify GL that we use our own program and buffers
	render_balls();

	glUseProgram(program);
	glBindVertexArray(floorVAO);
	mat4 model_matrix = mat4::translate(vec3(0, -1, 0)) * mat4::scale(vec3(2.0f, 2.0f, 2.0f));
	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	render_bricks();
	render_bars();

	if (ball.bColl)
	{
		engine->play2D(effect);
		ball.bColl = false;
	}
	//render_particle();


}

void reshape(GLFWwindow* window, int width, int height)
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

void print_help()
{
	printf("[help]\n");
	printf("- press ESC or 'q' to terminate the program\n");
	printf("- press F1 or 'h' to see help\n");
	printf("\n");
	printf("- press 'SPACE_BAR' to make size of the bar twice\n");
	printf("- Note: After pushing 'SPACE_BAR', size of the bar is going to be bigger.\n");
	printf("- But it will be smaller than origin size soon. That's penalty of using BIG_BAR.\n");
	printf("\n");
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_HOME)					cam = camera();
		else if (key == GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_SPACE)
		{
			if (!bar.bar_size_up_state)
			{
				bar.bar_size_up_state = !bar.bar_size_up_state;
				bar.size_up_chance--;
				bar.size_scale.x *= 2;
			}
		}
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		vec2 npos = vec2(float(pos.x) / float(window_size.x - 1), float(pos.y) / float(window_size.y - 1));
		if (action == GLFW_PRESS)			tb.begin(cam.view_matrix, npos.x, npos.y);
		else if (action == GLFW_RELEASE)	tb.end();
	}
}

void motion(GLFWwindow* window, double x, double y)
{
	if (!tb.b_tracking) return;
	vec2 npos = vec2(float(x) / float(window_size.x - 1), float(y) / float(window_size.y - 1));
	cam.view_matrix = tb.update(npos.x, npos.y);
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);	// set clear color
	glEnable(GL_CULL_FACE);								// turn on backface culling
	glEnable(GL_DEPTH_TEST);								// turn on depth tests
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	
	ball_init();

	// create corners and vertices
	vertex corners[4];
	corners[0].pos = vec3(-1.0f, 0.0f, +1.0f);	corners[0].tex = vec2(0.0f, 0.0f);
	corners[1].pos = vec3(+1.0f, 0.0f, +1.0f);	corners[1].tex = vec2(1.0f, 0.0f);
	corners[2].pos = vec3(+1.0f, 0.0f, -1.0f);	corners[2].tex = vec2(1.0f, 1.0f);
	corners[3].pos = vec3(-1.0f, 0.0f, -1.0f);	corners[3].tex = vec2(0.0f, 1.0f);
	vertex vertices[6] = { corners[0], corners[1], corners[2], corners[0], corners[2], corners[3] };

	// generation of vertex buffer is the same, but use vertices instead of corners
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &vertex_buffer_floor);

	glBindVertexArray(floorVAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_floor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	cg_bind_vertex_attributes(program);

	text_init();
	cube_init();
	bricks_init();
	brick_texture_init();
	//particle_init();

	engine = irrklang::createIrrKlangDevice();

	if (!engine) return false;

	bgm = engine->addSoundSourceFromFile(bgm_path);
	effect = engine->addSoundSourceFromFile(effect_path);

	//set default volume;
	bgm->setDefaultVolume(0.2f);
	effect->setDefaultVolume(0.2f);

	//play the sound file
	engine->play2D(bgm, true);

	return true;
}

void user_finalize()
{
	// close the engine
	engine->drop();
}

int main(int argc, char* argv[])
{
	if (!glfwInit()) { printf("[error] failed in glfwInit()\n"); return 1; }

	// create window and initialize OpenGL extensions
	if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) { glfwTerminate(); return 1; }
	if (!cg_init_extensions(window)) { glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return 1; }	// create and compile shaders/program
	if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback(window, reshape);	// callback for window resizing events
	glfwSetKeyCallback(window, keyboard);			// callback for keyboard events
	glfwSetMouseButtonCallback(window, mouse);	// callback for mouse click inputs
	glfwSetCursorPosCallback(window, motion);		// callback for mouse movement

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// enters rendering/event loop
	for (frame = 0; !glfwWindowShouldClose(window); frame++)
	{
		glfwPollEvents();	// polling and processing of events
		// clear screen (with background color) and clear depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render texts
		render_text("Hello text!", 100, 100, 1.0f, vec4(0.5f, 0.8f, 0.2f, 1.0f));
		render_text("I love Computer Graphics!", 100, 125, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f));
		render_text("Blinking text here", 100, 155, 0.6f, vec4(0.5f, 0.7f, 0.7f, 0.0f));

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Game"))
			{

				if (ImGui::MenuItem("Start"))
				{
					if (b_game_start)
					{
						ball_init();
						cube_init();
						bricks_init();
						brick_texture_init();
					}
					b_game_start = true;
					b_game_pause = false;
					b_die = false;
					frame = 0;
				}

				char* pauseState;

				if (b_game_pause)
					pauseState = "Resume";
				else
					pauseState = "Pause";

				if (ImGui::MenuItem(pauseState))
				{
					if (b_game_start)
					{
						b_game_pause = !b_game_pause;
						if (!b_game_pause)
							frame = 0;
					}
				}
					

				if (ImGui::MenuItem("Quit"))
					return 0;

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		if (!b_game_start)
		{
			glfwSwapBuffers(window);
			continue;
		}

		if (frame == 0)
		{
			pre_t = float(glfwGetTime());
			continue;
		}

		if (ball.out)
			b_die = true;

		if (!b_game_pause && !b_die)
		{
			cur_t = float(glfwGetTime());
			float diff_t = cur_t - pre_t;

			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				float dx = diff_t * bar.speed;
				bar.pos.x += dx;
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				float dx = diff_t * bar.speed;
				bar.pos.x -= dx;
			}

			ball.update(diff_t, bar, bricks);
			bar.update(diff_t);
			pre_t = cur_t;

			update();			// per-frame update
		}
	
		render();			// per-frame render

		glfwSwapBuffers(window);

	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}