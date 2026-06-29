#ifndef ZQLIGHTING_H
#define ZQLIGHTING_H

#include "ZQmodel.h"

typedef struct {
	dvec3_t position;
	vec3_t forward;
	vec2_t fov;			// 0.0 to 180.0
	float falloff;
	float range;
	float roundness;	// 0.0 to 1.0
} ZQdynamic_spot_light;

typedef struct {

} ZQambient_light;

#endif