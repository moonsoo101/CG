#pragma once
#ifndef __BAR_H__
#define __BAR_H__
#include "cgmath.h"
#include "cgut.h"

#define BAR_SIZE_UP_CHANCE_LIMIT 3

struct bar_t
{
	vec3    size_scale = vec3(1.0f / 8, 0.02f, 0.02f);		// radius
	vec3	pos = vec3(0, -0.8f, 0);		// 2D position for translation
	vec4	color = vec4(0, 0, 1.0f, 1.0f);				// RGBA color in [0,1]
	float	speed = 1.0f;
	mat4	model_matrix;		// modeling transformation

	int		size_up_chance = BAR_SIZE_UP_CHANCE_LIMIT;
	bool	bar_size_up_state = false;
	int		count = 0; // after bar gets bigger, how many times collisions occur.

	bar_t(float ball_radius)
	{
		size_scale.z = ball_radius;
	}
	// public functions
	void	update(float t);
	bool	isBarXOut();
	// bool	isOverlapWithAnother(const Bar_t& another);
};

inline bool bar_t::isBarXOut()
{
	float x_size = 2.0f * size_scale.x;
	bool x_out = (pos.x - (-1.0f) < x_size) || (1.0f - pos.x < x_size) ? true : false;
	return x_out;
}

inline void bar_t::update(float delta_t)
{
	if (bar_size_up_state && count == 2)
	{
		count = 0;
		bar_size_up_state = false;
		size_scale.x /= 2;
		size_scale.x *= (7.0f / 8);
	}
	
	mat4 scale_matrix =
	{
		size_scale.x, 0, 0, 0,
		0, size_scale.y, 0, 0,
		0, 0, size_scale.z, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, pos.x,
		0, 1, 0, pos.y,
		0, 0, 1, pos.z,
		0, 0, 0, 1
	};
	model_matrix = translate_matrix * scale_matrix;
}

#endif