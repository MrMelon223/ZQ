#ifndef ZQRENDER_CUH
#define ZQRENDER_CUH

#include "../../include/ZQlevel.h"
#include "../../include/ZQcamera.h"

#ifdef CUDA_RT

void getFrustumPlane(const glm::mat4&, vec4_t[6]);

__device__
bool isAABBinFrustum(const glm::vec4[6], glm::vec3, glm::vec3);

__global__
void compute_static_asset_visibility(vec4_t*, d_ZQmodel_c*, ZQasset_static*, ZQasset_static_instance*, ulong_t, bool*);

namespace CUDArender {
	void compute_visibility(ZQcamera*, d_ZQmodel_c*, ZQasset_static*, ZQasset_static_instance*, ulong_t, bool*);

	void cleanup_visibility(bool*);

	void cast_rays(ZQcamera*, d_ZQmodel_c*, ZQasset_static*, ZQasset_static_instance*, ulong_t, bool*);
}

#endif

#endif