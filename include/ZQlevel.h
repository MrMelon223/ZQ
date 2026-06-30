#ifndef ZQLEVEL_H
#define ZQLEVEL_H

#include "ZQshader.h"
#include "ZQtexture.h"

namespace Playerside {
	extern std::vector<ZQshader_program> shader_programs;
	static int shader_callback(void*, int, char**, char**);

	extern std::vector<ZQmodel> h_models;
	extern std::vector<d_ZQmodel> models;
	extern std::vector<d_ZQmodel_c> h_models_c;
	extern d_ZQmodel_c* d_models_c;
	static int model_callback(void*, int, char**, char**);

	//extern std::vector<ZQskeleton> skeletons;
	//static int skeleton_callback(void*, int, char**, char**);

	ulong_t find_model_idx(std::string);

	extern std::vector<ZQasset> static_assets;
	extern ZQasset* d_static_assets;
	static int asset_callback(void*, int, char**, char**);

	ulong_t find_asset_idx(std::string);

	extern std::vector<ZQtexture> textures;
	static int texture_callback(void*, int, char**, char**);

	ulong_t find_texture_idx(std::string);
}

namespace Level {
	extern std::vector<ZQmodel_instance> instances;

	extern std::vector<ZQasset_instance> static_instances;
	extern ZQasset_instance* d_static_instances;
}

class ZQlevel {
protected:
	std::string filepath;
	std::string name;
	std::string description;

	void load_from(std::string);

public:
	ZQlevel();
	ZQlevel(std::string);
};

typedef enum {
	SHADOW_DEFAULT		= 0,
	SHADOW_HORIZONTAL	= 1,
	SHADOW_VERTICAL		= 2,
	SHADOW_LARGE		= 3
} ShadowIdx;

namespace RenderSettings {
	extern double shadow_res_proportion;

	extern double viewport_res_hori;
	extern double viewport_res_vert;

	extern dim_t shadow_dims[4];
}

#endif
