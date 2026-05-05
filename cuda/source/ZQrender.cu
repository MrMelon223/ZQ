	// ZQrender.cu
#include "../include/ZQrender.cuh"

#ifdef CUDA_RT

void getFrustumPlane(const glm::mat4& viewProj, vec4_t planes[6]) {
	//glm::vec4 planes[6];

	// Extract rows for Gribb-Hartmann extraction
	glm::vec4 r1 = glm::row(viewProj, 0);
	glm::vec4 r2 = glm::row(viewProj, 1);
	glm::vec4 r3 = glm::row(viewProj, 2);
	glm::vec4 r4 = glm::row(viewProj, 3);

	planes[0] = r4 + r1; // Left
	planes[1] = r4 - r1; // Right
	planes[2] = r4 + r2; // Bottom
	planes[3] = r4 - r2; // Top
	planes[4] = r4 + r3; // Near
	planes[5] = r4 - r3; // Far

	// Normalize planes so we can calculate real distances
	for (int i = 0; i < 6; i++) {
		float length = glm::length(glm::vec3(planes[i]));
		planes[i] /= length;
	}

	//return planes;
}

__device__
bool isAABBinFrustum(const glm::vec4 planes[6], glm::vec3 min, glm::vec3 max) {
	for (int i = 0; i < 6; i++) {
		// Find the corner of the AABB furthest along the plane normal
		glm::vec3 positive = min;
		if (planes[i].x >= 0) positive.x = max.x;
		if (planes[i].y >= 0) positive.y = max.y;
		if (planes[i].z >= 0) positive.z = max.z;

		// If the "most positive" corner is behind the plane, the box is outside
		if (glm::dot(glm::vec3(planes[i]), positive) + planes[i].w < 0) {
			return false;
		}
	}
	return true;
}

__global__
void compute_static_asset_visibility(vec4_t* view_planes, d_ZQmodel_c* models, ZQasset_static* assets, ZQasset_static_instance* instances, ulong_t instance_count, bool* vis) {
	int_t j = blockDim.y * blockIdx.y + threadIdx.y,
		i = blockDim.x * blockIdx.x + threadIdx.x;
	ulong_t index = (ulong_t)j * blockDim.x + i;

	if (index < instance_count) {
		ZQasset_static_instance* inst = &instances[index];
		ZQasset_static* asset = &assets[inst->asset_idx];
		d_ZQmodel_c* model = &models[asset->lod0_idx];

		vec3_t aabb_max = model->aabb_min;
		vec3_t aabb_min = model->aabb_max;

		if (isAABBinFrustum(view_planes, aabb_min, aabb_max)) {
			vis[index] = true;
		}
		else {
			vis[index] = false;
		}
	}
}

void CUDArender::compute_visibility(ZQcamera* cam, d_ZQmodel_c* models, ZQasset_static* assets, ZQasset_static_instance* instances, ulong_t instance_count, bool* vis) {
	std::vector<ZQasset_static_instance> r = std::vector<ZQasset_static_instance>();

	mat4_t persp = glm::perspective(cam->fov.x, cam->fov.x / cam->fov.y, 0.01f, 100.0f);

	vec4_t view_planes[6],* d_view_planes;
	getFrustumPlane(persp, view_planes);

	cudaMalloc((void**)&vis, sizeof(bool) * instance_count);
	cudaMalloc((void**)&d_view_planes, sizeof(vec4_t) * 6);
	cudaMemcpy(d_view_planes, view_planes, sizeof(vec4_t) * 6, cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();

	compute_static_asset_visibility << <instance_count / 128, 128 >> > (d_view_planes, models, assets, instances, instance_count, vis);
	cudaDeviceSynchronize();
	
	cudaFree(d_view_planes);
}

void CUDArender::cleanup_visibility(bool* vis) {
	cudaFree(vis);
}

#endif