#pragma once
#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "cgmath.h"
#include "cgut.h"

struct camera
{
	vec3	eye = vec3(0, 0.1f, 3.6f);
//	vec3	eye = vec3(0, 0, 3.6f);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 1, 0);
	mat4	view_matrix = mat4::look_at(eye, at, up);

	float	fovy = PI / 4.0f; // must be in radian
	float	aspect_ratio;
	float	dNear = 1.0f;
	float	dFar = 1000.0f;
	mat4	projection_matrix;
};
#endif