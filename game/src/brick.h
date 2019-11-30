#pragma once
#ifndef __BRICK_H__
#define __BRICK_H__
#include "cgmath.h"
#include "cgut.h"

struct brick_t
{
	bool bShow;
	float x_scale, y_scale;
	float x_offset, y_offset;
	vec2 pos;
};

#endif