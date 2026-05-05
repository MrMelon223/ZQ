#ifndef ZQMODEL_H
#define ZQMODEL_H

#include "ZQcompute.h"
#include "ZQshader.h"

class ZQmodel;

struct d_ZQmodel {
	GLuint vPos;
	GLuint vNorms;
	GLuint vUVs;
	size_t v_count;

	GLuint tNorms;
	GLuint tIdxs;
	GLuint tShow;
	size_t t_count;

	GLuint vao;
};

//#ifdef CUDA_RT

const uint_t BVH_TRI_COUNT = 64;

struct ZQ_BVHbase {
	uint_t tri_idxs[BVH_TRI_COUNT];
	ushort_t tri_count;
	vec3_t min;
	vec3_t max;
};

struct ZQ_BVHnode {
	bool next_base;

	vec3_t min;
	vec3_t max;
	ulong_t next_a;
	ulong_t next_b;
};

struct d_ZQmodel_c {
	vec3_t* vPos;
	vec3_t* vNorms;
	uv_t* vUVs;
	size_t v_count;

	vec3_t* tNorms;
	tri_t* tIdxs;
	bool* tShow;
	size_t t_count;

	vec3_t aabb_min;
	vec3_t aabb_max;

	ZQ_BVHnode* bvh_nodes;
	ZQ_BVHbase* bvh_bases;
};

//#endif

struct ZQmodel_instance {
	ulong_t model_idx;

	dvec3_t position;
	vec3_t rotation;
};

struct ZQasset_static {
	std::string name;
	float scale;
	ulong_t lod0_idx;
	ulong_t lod1_idx;
	ulong_t lod2_idx;
	ulong_t lod3_idx;
	ulong_t material_idx;

};

struct ZQasset_static_instance {
	ulong_t asset_idx;
	dvec3_t position;
	vec3_t rotation;
};

ZQasset_static make_ZQasset(std::string, float, std::string, std::string, std::string, std::string, std::string);

class ZQmodel {
protected:
	std::string filepath;
	std::string name;

	std::vector<vec3_t> vPositions;
	std::vector<vec3_t> vNormals;
	std::vector<uv_t> vUVs;
	std::vector<vec2_t> vtPositions;
	std::vector<vec3_t> vtNormals;
	size_t vCount;

	std::vector<vec3_t> tNormals;
	std::vector<uint_t> tIndices;
	std::vector<vec3_t> tNormalsT;
	size_t tCount;

	vec3_t aabb[2];

	void load_data(const tinygltf::Model);

	void debug_print();

public:
	ZQmodel(std::string, std::string, bool);

	d_ZQmodel to_gpu();

	std::vector<vec3_t> get_vPositions() { return this->vPositions; }
	std::vector<vec3_t> get_vNormals() { return this->vNormals; }
	std::vector<vec3_t> get_tNormals() { return this->tNormals; }
	std::vector<uint_t> get_tri_idxs() { return this->tIndices; }
	std::string get_name() { return this->name; }

	ulong_t get_vCount() { return this->vCount; }
	ulong_t get_tCount() { return this->tCount; }

	void set_vtPos(vec2_t v, ulong_t idx) { this->vtPositions[idx] = v; }
	vec2_t get_vtPos(ulong_t idx) { return this->vtPositions[idx]; }
	void set_tNormalsT(vec3_t v, ulong_t idx) { this->tNormalsT[idx] = v; }

	vec3_t get_aabb_low() { return this->aabb[0]; }
	vec3_t get_aabb_high() { return this->aabb[1]; }
#ifdef CUDA_RT
	d_ZQmodel_c generate_model();
#endif
};

#endif
