#pragma once
#ifndef __BALL_H__
#define __BALL_H__
#include "cgmath.h"
#include "bar.h"
#include "brick.h"
#include <stdio.h>

struct ball_t
{
	float	radius = 0.02f;		// radius
	vec3	center = vec3(0, -0.5f, 0);		// 2D position for translation
	vec4	color = vec4(0, 0, 1.0f, 1.0f);				// RGBA color in [0,1]
	vec3	vel = vec3(0.8f, 0.8f, 1.2f);
	mat4	model_matrix;		// modeling transformation
	bool	bColl = false;
	bool	out = false;

	// public functions
	void	update(float t, bar_t& bar, std::vector<brick_t>& bricks);
	bool	isBallXOut();
	bool	isBallYOut();
	bool    isBallZOut();
	bool	isBallOut();
	bool	isOverlapWithBar(const bar_t& bar);
	void	checkOverlapWithBricks(std::vector<brick_t>& bricks);
};

inline bool ball_t::isBallXOut()
{
	bool x_out = (center.x - (-1.0f) < radius) || (1.0f - center.x < radius) ? true : false;
	return x_out;
}

inline bool ball_t::isBallYOut()
{
	bool y_out = (center.y - (-1.0f) < radius) || (1.0f - center.y < radius) ? true : false;
	return y_out;
}

inline bool ball_t::isBallZOut()
{
	bool z_out = (center.z - (-1.0f) < radius) || (1.0f - center.z < radius) ? true : false;
	return z_out;
}

inline bool ball_t::isBallOut()
{
	bool x_out = (center.x - (-1.0f) < radius) || (1.0f - center.x < radius) ? true : false;
	bool y_out = (center.y - (-1.0f) < radius) || (1.0f - center.y < radius) ? true : false;
	bool z_out = (center.z - (-1.0f) < radius) || (1.0f - center.z < radius) ? true : false;

	if ( x_out || y_out || z_out )
		return true;
	else
		return false;
}

inline bool ball_t::isOverlapWithBar(const bar_t& bar)
{
	float bar_x_size = 2.0f * bar.size_scale.x;
	float bar_y_size = 2.0f * bar.size_scale.y;
	float bar_y_offset = bar_y_size / 2;
	float bar_x_offset = bar_x_size / 2;

	vec2 barmin = vec2(bar.pos.x - bar_x_offset, bar.pos.y - bar_y_offset);
	vec2 barmax = vec2(bar.pos.x + bar_x_offset, bar.pos.y + bar_y_offset);

	vec2 ballmin = vec2(center.x - radius, center.y - radius);
	vec2 ballmax = vec2(center.x + radius, center.y + radius);

	bool overlaptest = (barmax.x < ballmin.x) || (ballmax.x < barmin.x) || (barmax.y < ballmin.y) || (ballmax.y < barmin.y);

	return !overlaptest;		
} 

inline void	ball_t::checkOverlapWithBricks(std::vector<brick_t>& bricks)
{
	for (unsigned int i = 0; i < bricks.size(); i++)
	{
		brick_t target_brick = bricks[i];
		if (!target_brick.bShow)
			continue;
		
		float target_brick_x_size = 2.0f * target_brick.x_scale;
		float target_brick_y_size = 2.0f * target_brick.y_scale;
		
		float target_brick_x_offset = target_brick_x_size / 2;
		float target_brick_y_offset = target_brick_y_size / 2;

		vec2 brickmin = vec2(target_brick.pos.x - target_brick_x_offset, target_brick.pos.y - target_brick_y_offset);
		vec2 brickmax = vec2(target_brick.pos.x + target_brick_x_offset, target_brick.pos.y + target_brick_y_offset);

		vec2 ballmin = vec2(center.x - radius, center.y - radius);
		vec2 ballmax = vec2(center.x + radius, center.y + radius);

		bool overlaptest = (brickmax.x < ballmin.x) || (ballmax.x < brickmin.x) || (brickmax.y < ballmin.y) || (ballmax.y < brickmin.y);

		if(!overlaptest)
		{
			bricks[i].bShow = false;
			vel.y *= -1;
			this->bColl = true;
		}
	}
}

inline void ball_t::update(float delta_t, bar_t& bar, std::vector<brick_t>& bricks)
{	

	if (isBallOut())
	{
		if (isBallXOut())
			vel.x *= -1;

		if (isBallYOut())
			out = true;

		if (isBallZOut())
			vel.z *= -1;
	}

	if (isOverlapWithBar(bar))
	{
		vel.y *= -1;
		if (bar.bar_size_up_state)
			bar.count++;
	}

	checkOverlapWithBricks(bricks);


	float d_x = vel.x * delta_t;
	float d_y = vel.y * delta_t;
	//float d_z = vel.z * delta_t;

	// update center.
	center.x += d_x;
	center.y += d_y;
	//center.z += d_z;

	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,			//center.z,
		0, 0, 0, 1
	};
	model_matrix = translate_matrix * scale_matrix;
}
#endif