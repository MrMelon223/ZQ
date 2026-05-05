#ifndef ZQLEVEL_H
#define ZQLEVEL_H

#include "ZQshader.h"
#include "ZQmodel.h"
#include "ZQtexture.h"

namespace Playerside {
	extern std::vector<ZQshader_program> shader_programs;
	static int shader_callback(void*, int, char**, char**);

	extern std::vector<ZQmodel> h_models;
	extern std::vector<d_ZQmodel> models;
	extern std::vector<d_ZQmodel_c> h_models_c;
	extern d_ZQmodel_c* d_models_c;
	static int model_callback(void*, int, char**, char**);

	ulong_t find_model_idx(std::string);

	extern std::vector<ZQasset_static> static_assets;
	extern ZQasset_static* d_static_assets;
	static int asset_callback(void*, int, char**, char**);

	ulong_t find_asset_idx(std::string);

	extern std::vector<ZQtexture> textures;
	static int texture_callback(void*, int, char**, char**);

	ulong_t find_texture_idx(std::string);

}

namespace Level {
	extern std::vector<ZQmodel_instance> instances;

	extern std::vector<ZQasset_static_instance> static_instances;
	extern ZQasset_static_instance* d_static_instances;
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

#endif
