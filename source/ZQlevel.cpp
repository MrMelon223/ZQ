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

	delete[] c;

	size_t light_len;
	c = new char[sizeof(size_t)];
	f.read(c, sizeof(size_t));
	memcpy(&light_len, c, sizeof(size_t));

	for (size_t i = 0; i < light_len; i++) {
		dvec3_t lPosition;
		vec3_t lForward;
		vec2_t lFov;
		float falloff;
		float range;
		float roundness;
		float intensity;
		color_t diff_color;
		color_t spec_color;
		uchar_t shad_type;

		char* ch = new char[sizeof(dvec3_t)];
		f.read(ch, sizeof(dvec3_t));
		memcpy(&lPosition, ch, sizeof(dvec3_t));
		delete[] ch;

		ch = new char[sizeof(vec3_t)];
		f.read(ch, sizeof(vec3_t));
		memcpy(&lForward, ch, sizeof(vec3_t));
		delete[] ch;

		ch = new char[sizeof(vec2_t)];
		f.read(ch, sizeof(vec2_t));
		memcpy(&lFov, ch, sizeof(vec2_t));
		delete[] ch;

		ch = new char[sizeof(float)];
		f.read(ch, sizeof(float));
		memcpy(&falloff, ch, sizeof(float));
		delete[] ch;

		ch = new char[sizeof(float)];
		f.read(ch, sizeof(float));
		memcpy(&range, ch, sizeof(float));
		delete[] ch;

		ch = new char[sizeof(float)];
		f.read(ch, sizeof(float));
		memcpy(&roundness, ch, sizeof(float));
		delete[] ch;

		ch = new char[sizeof(float)];
		f.read(ch, sizeof(float));
		memcpy(&intensity, ch, sizeof(float));
		delete[] ch;

		ch = new char[sizeof(color_t)];
		f.read(ch, sizeof(color_t));
		memcpy(&diff_color, ch, sizeof(color_t));
		delete[] ch;

		ch = new char[sizeof(color_t)];
		f.read(ch, sizeof(color_t));
		memcpy(&spec_color, ch, sizeof(color_t));
		delete[] ch;

		ch = new char[sizeof(uchar_t)];
		f.read(ch, sizeof(uchar_t));
		memcpy(&shad_type, ch, sizeof(uchar_t));
		delete[] ch;

		ShadowSize size = SHADOW_SMALL;

		switch (shad_type) {
		case 0:
			size = SHADOW_SMALL;
			break;
		case 1:
			size = SHADOW_MEDIUM;
			break;
		case 2:
			size = SHADOW_LARGE;
			break;
		case 3:
			size = SHADOW_MEGA;
			break;
		default:
			size = SHADOW_SMALL;
			break;
		}

		ZQdynamic_spot_light r = {lPosition, lForward, lFov, falloff, range, roundness, intensity, diff_color, spec_color, size, 0};

		long_t shad_width = RenderSettings::shadow_dims_hori[size] * RenderSettings::shadow_res_proportion;
		long_t shad_height = shad_width * (lFov.y / lFov.x) * RenderSettings::shadow_res_proportion;

		glGenTextures(1, &r.depth_map);
		glBindTexture(GL_TEXTURE_2D, r.depth_map);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, static_cast<size_t>(shad_width), static_cast<size_t>(shad_height), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

			// Framebuffer Object
		glGenFramebuffers(1, &r.depth_map_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, r.depth_map_fbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r.depth_map, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#ifdef DEBUG
			std::cout << "Error: Framebuffer is not complete!" << std::endl;
#endif
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
std::vector<ZQdynamic_spot_light> Level::spot_lights = std::vector<ZQdynamic_spot_light>();

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

long_t RenderSettings::shadow_dims_hori[4] = { 64, 96, 128, 256 };