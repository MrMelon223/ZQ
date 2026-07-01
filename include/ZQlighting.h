#ifndef ZQLIGHTING_H
#define ZQLIGHTING_H

#include "ZQhelper.h"

typedef enum {
	SHADOW_SMALL = 0,
	SHADOW_MEDIUM = 1,
	SHADOW_LARGE = 2,
	SHADOW_MEGA = 3
} ShadowSize;

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

	ShadowSize shadow_max_h_res;
	GLuint depth_map;
	GLuint depth_map_fbo;
} ZQdynamic_spot_light;

typedef struct {
	vec3_t forward;

	float intensity;
	color_t diffuse_color;
	color_t specular_color;
} ZQambient_light;

#endif