	// ZQlevel.cpp
#include "../include/ZQlevel.h"

std::vector<ZQshader_program> Playerside::shader_programs = std::vector<ZQshader_program>();

std::vector<ZQmodel> Playerside::h_models = std::vector<ZQmodel>();
std::vector<d_ZQmodel> Playerside::models = std::vector<d_ZQmodel>();
std::vector<d_ZQmodel_c> Playerside::h_models_c = std::vector<d_ZQmodel_c>();

//std::vector<ZQskeleton> Playerside::skeletons = std::vector<ZQskeleton>();

ulong_t Playerside::find_model_idx(std::string name) {
	for (ulong_t i = 0; i < Playerside::h_models.size(); i++) {
		if (Playerside::h_models[i].get_name() == name) {
			return i;
		}
	}
	return 0;
}

ulong_t Playerside::find_asset_idx(std::string name) {
	for (ulong_t i = 0; i < Playerside::h_models.size(); i++) {
		if (Playerside::static_assets[i].name == name) {
			return i;
		}
	}
	return 0;
}

std::vector<ZQtexture> Playerside::textures = std::vector<ZQtexture>();

ulong_t Playerside::find_texture_idx(std::string name) {
	for (ulong_t i = 0; i < Playerside::textures.size(); i++) {
		if (Playerside::textures[i].get_name() == name) {
			return i;
		}
	}
	return 0;
}

std::vector<ZQmodel_instance> Level::instances = std::vector<ZQmodel_instance>();
std::vector<ZQasset_instance> static_instances = std::vector<ZQasset_instance>();

void ZQlevel::load_from(std::string path) {
	std::ifstream f(path.c_str(), std::ios::in | std::ios::binary);
	if (!f.is_open()) {
#ifdef DEBUG
		std::cout << "Cannot find level at " << path << std::endl;
#endif
		return;
	}

	char* c = new char[sizeof(size_t)];
	size_t name_len = 0;

	f.read(c, sizeof(size_t));
	memcpy(&name_len, c, sizeof(size_t));
#ifdef DEBUG
	std::cout << "Name length: " << name_len << std::endl;
#endif
	delete[] c;

	this->name = "";
	for (size_t i = 0; i < name_len; i++) {
		char ch;
		f.get(ch);
		this->name.push_back(ch);
	}

#ifdef DEBUG
	std::cout << "Name: " << this->name << std::endl;
#endif

	size_t desc_len = 0;
	c = new char[sizeof(size_t)];
	f.read(c, sizeof(size_t));
	memcpy(&desc_len, c, sizeof(size_t));
#ifdef DEBUG
	std::cout << "Description length: " << desc_len << std::endl;
#endif
	delete[] c;

	this->description = "";
	for (size_t i = 0; i < desc_len; i++) {
		char ch;
		f.get(ch);
		this->description.push_back(ch);
	}

#ifdef DEBUG
	std::cout << "Description: " << this->description << std::endl;
#endif

	size_t asset_len;
	c = new char[sizeof(size_t)];
	f.read(c, sizeof(size_t));
	memcpy(&asset_len, c, sizeof(size_t));

	for (size_t i = 0; i < asset_len; i++) {
		dvec3_t pos;
		vec3_t rot;
		std::string name;
		size_t name_len;

		char* ch = new char[sizeof(glm::dvec3)];
		f.read(ch, sizeof(glm::dvec3));
		memcpy(&pos, ch, sizeof(glm::dvec3));

		f.read(ch, sizeof(glm::vec3));
		memcpy(&rot, ch, sizeof(glm::vec3));
		delete[] ch;

		ch = new char[sizeof(size_t)];
		f.read(ch, sizeof(size_t));
		memcpy(&name_len, ch, sizeof(size_t));
		delete[] ch;

		for (size_t i = 0; i < name_len; i++) {
			char cha;
			f.get(cha);
			name.push_back(cha);
		}
		ZQasset_instance aInst{};

		aInst.asset_idx = Playerside::find_asset_idx(name);
		aInst.position = pos;
		aInst.rotation = rot;

		Level::static_instances.push_back(aInst);
#ifdef DEBUG
		std::cout << "Static model #" << aInst.asset_idx << " added." << std::endl;
#endif
	}

#ifdef CUDA_RT
	cudaMalloc((void**)&Level::d_static_instances, sizeof(ZQasset_static_instance) * Level::static_instances.size());

	cudaMemcpy(Level::d_static_instances, Level::static_instances.data(), sizeof(ZQasset_static_instance) * Level::static_instances.size(), cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
#endif
}

std::vector<ZQasset> Playerside::static_assets = std::vector<ZQasset>();
std::vector<ZQasset_instance> Level::static_instances = std::vector<ZQasset_instance>();
ZQasset_instance* Level::d_static_instances;

ZQasset make_ZQasset(std::string name, float scale, std::string lod0name, std::string lod1name, std::string lod2name, std::string lod3name, std::string materialname) {
	ZQasset r{};
	r.scale = scale;
	r.name = name;

	r.lod0_idx = Playerside::find_model_idx(lod0name);
	r.lod1_idx = Playerside::find_model_idx(lod1name);
	r.lod2_idx = Playerside::find_model_idx(lod2name);
	r.lod3_idx = Playerside::find_model_idx(lod3name);

	r.material_idx = Playerside::find_texture_idx(materialname);

	return r;
}

ZQlevel::ZQlevel() {

}

ZQlevel::ZQlevel(std::string path) {
	this->filepath = path;

	this->load_from(this->filepath);
}

double RenderSettings::shadow_res_proportion = 1.0;

double RenderSettings::viewport_res_hori = 1.0;
double RenderSettings::viewport_res_vert = 1.0;

dim_t RenderSettings::shadow_dims[4] = { dim_t(64, 64),
										dim_t(128, 64),
										dim_t(64, 128),
										dim_t(128, 128) };