#pragma once
#ifndef __PARTICLE_H__
#define __PARTICLE_H__
#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "myrandom.h"

//*************************************
struct particle_t
{
	vec3 pos;
	vec4 color;
	vec3 velocity;
	float scale;
	float life;
	mat4 model_matrix;

	//optional
	float elapsed_time;
	float time_interval;

	particle_t() { reset(); }

	void reset()
	{
		pos = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f));
		color = vec4(random_range(0, 1.0f), random_range(0, 1.0f), random_range(0, 1.0f), 1);
		scale = random_range(0.0001f, 0.02f);
		life = random_range(0.01f, 1.0f);
		velocity = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f)) * 0.003f;
		elapsed_time = 0.0f;
		time_interval = random_range(200.0f, 600.0f);
	}

	void update()
	{
		const float dwTime = (float)glfwGetTime();
		elapsed_time += dwTime;

		if (elapsed_time > time_interval)
		{
			const float theta = random_range(0, 1.0f) * PI * 2.0f;
			constexpr float velocity_factor = 0.003f;
			velocity = vec3(cos(theta), sin(theta), cos(theta)) * velocity_factor;

			elapsed_time = 0.0f;
		}

		pos += velocity;

		constexpr float life_factor = 0.001f;
		life -= life_factor * dwTime;

		// disappear
		if (life < 0.0f)
		{
			constexpr float alpha_factor = 0.001f;
			color.a -= alpha_factor * dwTime;
		}

		// dead
		if (color.a < 0.0f) reset();

		mat4 scale_matrix =
		{
			scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale, 0,
			0, 0, 0, 1
		};

		mat4 translate_matrix =
		{
			1, 0, 0, pos.x,
			0, 1, 0, pos.y,
			0, 0, 1, pos.z,			//center.z,
			0, 0, 0, 1
		};
		model_matrix = translate_matrix * scale_matrix;
	}
};
#endif