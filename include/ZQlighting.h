#ifndef ZQLIGHTING_H
#define ZQLIGHTING_H

#include "ZQhelper.h"

typedef struct {
	dvec3_t position;
	vec3_t forward;
	vec2_t fov;			// 0.0 to 180.0
	float falloff;
	float range;
	float roundness;	// 0.0 to 1.0

	float intensity;
	color_t diffuse_color;
	color_t specular_color;

	GLuint depth_map;
} ZQdynamic_spot_light;

typedef struct {
	vec3_t forward;

	float intensity;
	color_t diffuse_color;
	color_t specular_color;
} ZQambient_light;

#endif