#pragma once
#ifndef __MY_RANDOM_H__
#define __MY_RANDOM_H__
#include "cgmath.h"
#include "cgut.h"
//*************************************
//Random Func
inline const float random_range(float min, float max) { return min + rand() / (RAND_MAX / (max - min)); }
#endif