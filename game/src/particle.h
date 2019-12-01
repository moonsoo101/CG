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
	bool bDead=false;

	particle_t() { reset(); }
	void reset()
	{
		pos = vec3(0, 0, 0);
		color = vec4(random_range(0, 1.0f), random_range(0, 1.0f), random_range(0, 1.0f), 1);
		scale = random_range(0.003f, 0.004f);

		life = random_range(0.01f, 1.0f);
		velocity = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 0.0f), random_range(-1.0f, 1.0f)) * 0.003f;
	}

	// dead -> return true;
	void update()
	{
		const float dwTime = (float)glfwGetTime();
		
		pos += velocity;

		//constexpr float life_factor = 0.001f;
		constexpr float life_factor = 1.0f;
		life -= life_factor * dwTime;

		// disappear
		if (life < 0.0f)
		{
			constexpr float alpha_factor = 0.002f;
			color.a -= alpha_factor * dwTime;
		}

		// dead
		if (color.a < 0.0f) 
		{
			reset();
			bDead = true;
		}
			

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