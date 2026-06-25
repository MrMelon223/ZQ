#ifndef ZQCAMERA_H
#define ZQCAMERA_H

#include "ZQhelper.h"

extern float CTRL_DEADZONE_MOVE;
extern float CTRL_MOVE_MULTIPLIER;

#ifdef CUDA_RT

typedef struct {
	bool hit;
	uv_t uv;

	ulong_t model_idx;
	uint_t tri_idx;

	float depth;
} ZQray_payload_t;

typedef struct {
	vec3_t position;
	vec3_t direction;

	ZQray_payload_t payload;
} ZQray_t;

#endif

typedef struct {
	dvec3_t position;
	vec3_t rotation;
	vec2_t fov;
	dim_t dims;

#ifdef CUDA_RT
	ZQray_t* rays;
#endif
} ZQcamera;

void cursor_input(ZQcamera*, dim_t, double, double);

void controller_input(ZQcamera*, dim_t);

void controller_view(ZQcamera*, float, float);

void controller_move(ZQcamera*, float, float);

#endif