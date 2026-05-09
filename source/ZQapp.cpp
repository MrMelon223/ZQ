	// ZQapp.cpp
#include "../include/ZQapp.h"

int Playerside::model_callback(void* data, int argc, char** argv, char** azColName) {
	std::string path = argv[0];
	std::string name = argv[1];
#ifdef DEBUG
	std::cout << path << ", " << name << std::endl;
#endif
	Playerside::h_models.push_back(ZQmodel(path, name, true));
	Playerside::models.push_back(Playerside::h_models.back().to_gpu());
#ifdef CUDA_RT
	Playerside::h_models_c.push_back(Playerside::h_models.back().generate_model());
#endif
	return 0;
}

int Playerside::shader_callback(void* data, int argc, char** argv, char** azColName) {
	std::string vs_path = argv[0];
	std::string ps_path = argv[1];
	std::string name = argv[2];
#ifdef DEBUG
	std::cout << vs_path << ", " << ps_path << ", " << name << std::endl;
#endif
	Playerside::shader_programs.push_back(ZQshader_program(vs_path, ps_path, name));
	return 0;
}

std::queue<KBdUse> PlayerCtrl::keyboard_queue;

/*void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	KBdUse u = { key, scancode, action, mode };

	PlayerCtrl::keyboard_queue.push(u);
}*/


void ZQapp::load_shaders() {
	sqlite3* db;
	int exit = 0;
	exit = sqlite3_open("resources/ZQmaster.db", &db);
	if (exit) {
#ifdef DEBUG
		std::cout << "cannot find master database " << std::endl;
#endif
		return;
	}

	std::string command = "SELECT * FROM shaders;";
	std::string data;
	int rc = sqlite3_exec(db, command.c_str(), Playerside::shader_callback, (void*)data.c_str(), NULL);
	if (rc != SQLITE_OK) {
#ifdef DEBUG
		std::cout << "shaders db return: " << rc << std::endl;
#endif
		return;
	}

	sqlite3_close(db);
}

void ZQapp::load_models() {
	sqlite3* db;
	int exit = 0;
	exit = sqlite3_open("resources/ZQmaster.db", &db);
	if (exit) {
#ifdef DEBUG
		std::cout << "cannot find master database " << std::endl;
#endif
		return;
	}

	std::string command = "SELECT * FROM models;";
	std::string data;
	int rc = sqlite3_exec(db, command.c_str(), Playerside::model_callback, (void*)data.c_str(), NULL);
	if (rc != SQLITE_OK) {
#ifdef DEBUG
		std::cout << "models db return: " << rc << std::endl;
#endif
		return;
	}

#ifdef CUDA_RT
	cudaMalloc((void**)&Playerside::d_models_c, sizeof(d_ZQmodel_c) * Playerside::h_models_c.size());

	cudaMemcpy(Playerside::d_models_c, Playerside::h_models_c.data(), sizeof(d_ZQmodel_c) * Playerside::h_models_c.size(), cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
#endif

	sqlite3_close(db);
}

int Playerside::asset_callback(void* data, int argc, char** argv, char** azColName) {
	std::string name = argv[0];
	std::string scale = argv[1];
	std::stringstream scl_in(scale);
	float scale_flt;

	scl_in >> scale_flt;

	std::string lod0 = argv[2];
	std::string lod1 = argv[3];
	std::string lod2 = argv[4];
	std::string lod3 = argv[5];
	std::string material = argv[6];

#ifdef DEBUG
	std::cout << "Asset: " << name << std::endl;
#endif
	Playerside::static_assets.push_back(make_ZQasset(name, scale_flt, lod0, lod1, lod2, lod3, material));
	return 0;
}

int Playerside::texture_callback(void* data, int argc, char** argv, char** azColName) {
	std::string path = argv[0];
	std::string name = argv[1];

#ifdef DEBUG
	std::cout << "Texture: " << name << std::endl;
#endif
	Playerside::textures.push_back(ZQtexture(path, name));
	return 0;
}
d_ZQmodel_c* Playerside::d_models_c;
ZQasset_static* Playerside::d_static_assets;

void ZQapp::load_assets() {
	sqlite3* db;
	int exit = 0;
	exit = sqlite3_open("resources/ZQmaster.db", &db);
	if (exit) {
#ifdef DEBUG
		std::cout << "cannot find master database " << std::endl;
#endif
		return;
	}

	std::string command = "SELECT * FROM assets;";
	std::string data;
	int rc = sqlite3_exec(db, command.c_str(), Playerside::asset_callback, (void*)data.c_str(), NULL);
	if (rc != SQLITE_OK) {
#ifdef DEBUG
		std::cout << "assets db return: " << rc << std::endl;
#endif
		return;
	}

#ifdef CUDA_RT
	cudaMalloc((void**)&Playerside::d_static_assets, sizeof(ZQasset_static) * Playerside::static_assets.size());

	cudaMemcpy(Playerside::d_static_assets, Playerside::static_assets.data(), sizeof(ZQasset_static) * Playerside::static_assets.size(), cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
#endif

	sqlite3_close(db);
}

void ZQapp::load_textures() {
	sqlite3* db;
	int exit = 0;
	exit = sqlite3_open("resources/ZQmaster.db", &db);
	if (exit) {
#ifdef DEBUG
		std::cout << "cannot find master database " << std::endl;
#endif
		return;
	}

	std::string command = "SELECT * FROM textures;";
	std::string data;
	int rc = sqlite3_exec(db, command.c_str(), Playerside::texture_callback, (void*)data.c_str(), NULL);
	if (rc != SQLITE_OK) {
#ifdef DEBUG
		std::cout << "assets db return: " << rc << std::endl;
#endif
		return;
	}

#ifdef CUDA_RT
	cudaMalloc((void**)&Playerside::d_static_assets, sizeof(ZQasset_static) * Playerside::static_assets.size());

	cudaMemcpy(Playerside::d_static_assets, Playerside::static_assets.data(), sizeof(ZQasset_static) * Playerside::static_assets.size(), cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
#endif

	sqlite3_close(db);
}

void ZQapp::process_KBdUse(KBdUse u) {
	/*if (u.key == GLFW_KEY_ESCAPE && u.action == GLFW_PRESS) {
		this->loop = false;
	}
	if (u.key == GLFW_KEY_W && u.action == GLFW_PRESS) {
		this->camera.position += dvec3_t(glm::normalize(glm::radians(this->camera.rotation) / 2.0f * 3.1415f)) * (double)this->last_time;
	}
	if (u.key == GLFW_KEY_S && u.action == GLFW_PRESS) {
		this->camera.position -= dvec3_t(glm::normalize(glm::radians(this->camera.rotation) / 2.0f * 3.1415f)) * (double)this->last_time;
	}
	if (u.key == GLFW_KEY_D && u.action == GLFW_PRESS) {
		this->camera.position -= dvec3_t(glm::normalize(glm::cross(glm::radians(this->camera.rotation) / 2.0f * 3.1415f, glm::radians(this->camera.rotation) / 2.0f * 3.1415f))) * (double)this->last_time;
	}
	if (u.key == GLFW_KEY_A && u.action == GLFW_PRESS) {
		this->camera.position -= dvec3_t(glm::normalize(-glm::cross(glm::radians(this->camera.rotation) / 2.0f * 3.1415f, glm::radians(this->camera.rotation) / 2.0f * 3.1415f))) * (double)this->last_time;
	}*/
}

void ZQapp::process_CtrlEvent(SDL_Event e) {
	if (e.type == SDL_QUIT) {
		this->loop = false;
	}
}

void ZQapp::empty_CtrlQueues() {
	for (ulong_t i = PlayerCtrl::keyboard_queue.size(); i > 0; i--) {
		KBdUse u = PlayerCtrl::keyboard_queue.front();
		PlayerCtrl::keyboard_queue.pop();

		this->process_KBdUse(u);
	}
}

std::vector<glm::vec4> getFrustumPlanes(const glm::mat4& viewProj) {
	std::vector<glm::vec4> planes(6);

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

	return planes;
}

bool isBoxInFrustum(const std::vector<glm::vec4>& planes, glm::vec3 min, glm::vec3 max) {
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

std::vector<ZQasset_static_instance> ZQapp::compute_visibility(ZQcamera* cam) {
	std::vector<ZQasset_static_instance> r = std::vector<ZQasset_static_instance>();

	mat4_t persp = glm::perspective(cam->fov.x, cam->fov.x / cam->fov.y, 0.01f, 1000.0f);

	std::vector<vec4_t> view_planes = getFrustumPlanes(persp);

	for (ulong_t i = 0; i < Level::static_instances.size(); i++) {
		ZQasset_static_instance* inst = &Level::static_instances[i];
		ZQasset_static* asset = &Playerside::static_assets[inst->asset_idx];
		ZQmodel* model = &Playerside::h_models[asset->lod0_idx];

		vec3_t aabb_max = vec3_t(cam->position) - vec3_t(inst->position) + model->get_aabb_high();
		vec3_t aabb_min = vec3_t(cam->position) - vec3_t(inst->position) + model->get_aabb_low();

		if (isBoxInFrustum(view_planes, aabb_min, aabb_max)) {
			if (r.empty()) {
				r.push_back(*inst);
			}
			else {
				float dist_local = glm::length(inst->position - cam->position);
				float cmp = glm::length(r.back().position - cam->position);

				if (dist_local < cmp) {
					r.push_back(*inst);
				}
				else if (dist_local >= cmp) {
					float cmp_begin = glm::length(r.front().position - cam->position);
					if (dist_local > cmp_begin) {
						r.insert(r.begin(), *inst);
					}
					else if (dist_local < cmp_begin && dist_local >= cmp) {
						for (ulong_t j = 1; j < r.size(); j++) {
							float cmp_loc = glm::length(r[j].position - cam->position);
							if (dist_local < cmp_loc) {
								r.insert(r.begin() + (j - 1), *inst);
								break;
							}
							else if (j == r.size() - 1) {
								r.push_back(*inst);
								break;
							}
						}
					}
				}
			}
		}
	}

	return r;
}

ZQapp::ZQapp(ZQcompute* c, std::string name, dim_t dims, bool fullscreen, short_t monitor) {
	this->compute = c;

	this->name = name;
	this->dims = dims;
	this->fullscreen = fullscreen;
	this->monitor = monitor;

	this->control_type = Controller;

	/*if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
		this->control_type = Controller;
	}
	else {
		this->control_type = KBdMouse;
	}(*/
}

void ZQapp::init() {
#ifdef CUDA_RT
	cudaSetDevice(0);
#endif
	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
#ifdef DEBUG
		std::cout << "Cannot initialize SDL!" << std::endl;
#endif
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (SDL_NumJoysticks() < 1) {
#ifdef DEBUG
		std::cout << "No joysticks connected!" << std::endl;
#endif
		return;
	}

	if (SDL_NumJoysticks() < 2) {
#ifdef DEBUG
		std::cout << "Not enough joysticks!" << std::endl;
#endif
		return;
	}

	joystick_left = SDL_JoystickOpen(0);
	joystick_right = SDL_JoystickOpen(1);

	if (this->fullscreen) {
		this->win = SDL_CreateWindow(this->name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else {
		this->win = SDL_CreateWindow(this->name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, this->dims.x, this->dims.y, 0);
	}

	if (this->win == NULL) {
#ifdef DEBUG
		std::cout << "Cannot create window: " << this->name << std::endl;
#endif
		SDL_Quit();
		return;
	}

	SDL_GLContext context = SDL_GL_CreateContext(this->win);
	if (!gladLoadGLLoader((GLADloadproc)context)) {
#ifdef DEBUG
		std::cout << "Cannot load OpenGL [glad.h]" << std::endl;
#endif
		return;
	}

	glViewport(0, 0, this->dims.x, this->dims.y);
	this->camera = ZQcamera{ dvec3_t(0.0), vec3_t(0.0f, 0.0f, 0.0f), vec2_t(90.0f, 90.0f * ((float)this->dims.x) / this->dims.y), this->dims };
	glDisable(GL_CULL_FACE);

	this->load_shaders();
	this->load_models();
	this->load_textures();
	this->load_assets();

	this->level = new ZQlevel("resources/levels/test_level.zqlbin");

	this->last_time = 0.0f;
}

void ZQapp::main_loop() {
	this->loop = true;
	while (this->loop) {

		double cursorX, cursorY;

		this->empty_CtrlQueues();

		if (this->control_type == KBdMouse) {
			//glfwGetCursorPos(this->win, &cursorX, &cursorY);
			//glfwSetCursorPos(this->win, (double)(this->dims.x / 2), (double)(this->dims.y / 2));
			//cursor_input(&this->camera, this->dims, cursorX, cursorY);
		}
		else if (this->control_type == Controller) {
			SDL_Event event;

			while (SDL_PollEvent(&event)) {
				this->process_CtrlEvent(event);
			}
			float lx_axis = (float)SDL_JoystickGetAxis(this->joystick_left, 0) / 32768;
			float ly_axis = (float)SDL_JoystickGetAxis(this->joystick_left, 1) / 32768;

			controller_view(&this->camera, lx_axis, ly_axis);
		}

		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDepthFunc(GL_LESS);
#ifdef CUDA_RT

		bool* vis = 0;
		CUDArender::compute_visibility(&this->camera, Playerside::d_models_c, Playerside::d_static_assets, Level::d_static_instances, Level::static_instances.size(), vis);

		CUDArender::cleanup_visibility(vis);
#endif
#ifndef CUDA_RT
		std::vector<ZQasset_static_instance> static_list = compute_visibility(&this->camera);
		for (ulong_t i = 0; i < static_list.size(); i++) {
			draw(&this->camera, &static_list[i], &Playerside::shader_programs[0]);
		}
		//draw(&Playerside::models[2], &Playerside::shader_programs[0]);
#endif
		ulong_t start = SDL_GetPerformanceCounter();
		ulong_t freq = SDL_GetPerformanceFrequency(); // Ticks per second

		this->last_time = ((double)start / (double)freq) - this->last_time;

		SDL_GL_SwapWindow(this->win);
		//glfwPollEvents();
	}
}

void draw(ZQcamera* cam, ZQasset_static_instance* mod, ZQshader_program* prog) {
	ZQasset_static* asset = &Playerside::static_assets[mod->asset_idx];
#ifdef DEBUG
	std::cout << "Camera rot: { " << cam->rotation.x << ", " << cam->rotation.y << " }" << std::endl;
#endif
	d_ZQmodel* d_m;
	ZQmodel* modl;

	float dist = glm::length(mod->position - cam->position);

	if (dist < 10.0f) {
		d_m = &Playerside::models[asset->lod0_idx];
		modl = &Playerside::h_models[asset->lod0_idx];
	}
	else if (dist < 20.0f) {
		d_m = &Playerside::models[asset->lod1_idx];
		modl = &Playerside::h_models[asset->lod1_idx];
	}
	else if (dist < 30.0f) {
		d_m = &Playerside::models[asset->lod2_idx];
		modl = &Playerside::h_models[asset->lod2_idx];
	}
	else {
		d_m = &Playerside::models[asset->lod3_idx];
		modl = &Playerside::h_models[asset->lod3_idx];
	}

	GLuint progam = prog->get_program();
	int_t matGeo = glGetUniformLocation(progam, "matGeo");
	int_t matVP = glGetUniformLocation(progam, "matVP");
	int_t matCam = glGetUniformLocation(progam, "matCam");

	int_t color_map = glGetUniformLocation(progam, "color_map");

	mat4_t model = mat4_t(1.0f);
	model = glm::translate(model, vec3_t(mod->position));
	model = glm::rotate(model, glm::radians(mod->rotation.x), vec3_t(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(mod->rotation.y), vec3_t(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(mod->rotation.z), vec3_t(0.0f, 0.0f, 1.0f));

	mat4_t geo = mat4_t(1.0f);
	geo = glm::translate(geo, -vec3_t(cam->position));
	geo = glm::rotate(geo, glm::radians(cam->rotation.x), vec3_t(1.0f, 0.0f, 0.0f));
	geo = glm::rotate(geo, glm::radians(cam->rotation.y), vec3_t(0.0f, 1.0f, 0.0f));
	//geo = glm::rotate(geo, glm::radians(0.0f), vec3_t(0.0f, 0.0f, 1.0f));

	geo = glm::inverse(geo);

	mat4_t persp = glm::perspective(glm::radians(cam->fov.x), (float)cam->dims.x / cam->dims.y, 0.1f, 1000.0f);

	glUseProgram(progam);

	vec3_t mLoc = vec3_t(mod->position - cam->position);

	const GLfloat* mPtr = glm::value_ptr(model);
	const GLfloat* gPtr = glm::value_ptr(geo);
	const GLfloat* vPtr = glm::value_ptr(persp);
	const GLfloat* lPtr = glm::value_ptr(mLoc);

	glUniformMatrix4fv(matGeo, 1, GL_FALSE, mPtr);
	glUniformMatrix4fv(matVP, 1, GL_FALSE, vPtr);
	glUniformMatrix4fv(matCam, 1, GL_FALSE, gPtr);

	glUniform1i(color_map, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Playerside::textures[asset->material_idx].get_texture());

	glBindVertexArray(d_m->vao);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod->tIdxs);
	//std::vector<tri_t> tris = modl->get_tri_idxs();
	glDrawElements(GL_TRIANGLES, modl->get_tri_idxs().size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}