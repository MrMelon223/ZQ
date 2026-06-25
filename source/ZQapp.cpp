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

//void ZQapp::process_KBdUse(KBdUse u) {
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
//}

void ZQapp::process_KBdDown(SDL_Scancode& k) {
	switch (k) {
	case SDL_SCANCODE_ESCAPE:
		this->loop = false;
		break;
	case SDL_SCANCODE_W:
		camera_forward(&this->camera, 1.0);
		break;
	case SDL_SCANCODE_S:
		camera_backward(&this->camera, 1.0);
		break;
	}
}

void ZQapp::process_KBdHold(SDL_Scancode& k) {
	switch (k) {
	case SDL_SCANCODE_ESCAPE:
		this->loop = false;
		break;
	case SDL_SCANCODE_W:
		camera_forward(&this->camera, 1.0);
		break;
	case SDL_SCANCODE_S:
		camera_backward(&this->camera, 1.0);
		break;
	}
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

		//this->process_KBdUse(u);
	}
}

std::vector<glm::vec4> getFrustumPlanes(const glm::mat4& viewProj) {
	std::vector<glm::vec4> planes(6);

	glm::vec4 r1 = glm::row(viewProj, 0);
	glm::vec4 r2 = glm::row(viewProj, 1);
	glm::vec4 r3 = glm::row(viewProj, 2);
	glm::vec4 r4 = glm::row(viewProj, 3);

	planes[0] = r4 + r1;
	planes[1] = r4 - r1;
	planes[2] = r4 + r2;
	planes[3] = r4 - r2;

	planes[4] = r4 + r3;

	planes[5] = r4 - r3; // Far

	for (int i = 0; i < 6; i++) {
		float length = glm::length(glm::vec3(planes[i]));
		if (length > 0.0f) {
			planes[i] /= length;
		}
	}

	return planes;
}

bool isBoxInFrustum(const std::vector<glm::vec4>& planes, glm::vec3 min, glm::vec3 max) {
	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 extents = max - center;

	for (int i = 0; i < 6; i++) {
		const glm::vec4& plane = planes[i];
		glm::vec3 normal = glm::vec3(plane);

		float radius = extents.x * std::abs(normal.x) +
			extents.y * std::abs(normal.y) +
			extents.z * std::abs(normal.z);


		float distance = glm::dot(normal, center) + plane.w;

		if (distance < -radius) {
			return false;
		}
	}

	// The box overlaps or is entirely inside the frustum volume
	return true;
}

std::vector<ZQasset_static_instance> ZQapp::compute_visibility(ZQcamera* cam) {
	std::vector<ZQasset_static_instance> r = std::vector<ZQasset_static_instance>();

	float pitch = glm::radians(cam->rotation.x);
	float yaw = glm::radians(cam->rotation.y);
	glm::vec3 forward(
		cos(pitch) * sin(yaw),
		sin(pitch),
		-cos(pitch) * cos(yaw)
	);
	mat4_t geo = glm::lookAt(vec3_t(cam->position), vec3_t(cam->position) + forward, glm::vec3(0.0f, 1.0f, 0.0f));
	mat4_t persp = glm::perspective(glm::radians(cam->fov.x), (float)(cam->dims.x) / cam->dims.y, 0.01f, 10000.0f);

	mat4_t viewProj = persp * geo;

	std::vector<vec4_t> view_planes = getFrustumPlanes(viewProj);

	for (ulong_t i = 0; i < Level::static_instances.size(); i++) {
		ZQasset_static_instance* inst = &Level::static_instances[i];
		ZQasset_static* asset = &Playerside::static_assets[inst->asset_idx];
		ZQmodel* model = &Playerside::h_models[asset->lod0_idx];

		vec3_t aabb_min = vec3_t(inst->position) + model->get_aabb_low();
		vec3_t aabb_max = vec3_t(inst->position) + model->get_aabb_high();

		vec3_t real_min = glm::min(aabb_min, aabb_max);
		vec3_t real_max = glm::max(aabb_min, aabb_max);

		if (isBoxInFrustum(view_planes, real_min, real_max)) {
			r.push_back(*inst);
			/*if (r.empty()) {
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
			}*/
		}
	}

	std::sort(r.begin(), r.end(), [cam](const ZQasset_static_instance& a, const ZQasset_static_instance& b) {
		vec3_t distA = vec3_t(a.position - cam->position);
		vec3_t distB = vec3_t(b.position - cam->position);

		return glm::dot(distA, distA) < glm::dot(distB, distB);
	});

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

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
#ifdef DEBUG
		std::cout << "Cannot initialize SDL!: " << SDL_GetError() << std::endl;
#endif
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	int joystick_count = SDL_NumJoysticks();
	if (joystick_count == 0) {
#ifdef DEBUG
		std::cout << "No joysticks connected!" << std::endl;
#endif
		this->control_type = KBdMouse;
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else {
		this->control_type = Controller;
		this->joystick_left = SDL_GameControllerOpen(0);
	}

	/*if (SDL_IsGameController(0)) {
		joystick_left = SDL_GameControllerOpen(0); // <-- USE THIS INSTEAD
		this->control_type = Controller;
	}
	else {
#ifdef DEBUG
		std::cout << "Connected device is not a standard Game Controller layout!" << std::endl;
#endif
		this->control_type = KBdMouse;
	}*/

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (this->fullscreen) {
		this->win = SDL_CreateWindow(this->name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
	}
	else {
		this->win = SDL_CreateWindow(this->name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, this->dims.x, this->dims.y, SDL_WINDOW_OPENGL);
	}

	if (this->win == NULL) {
#ifdef DEBUG
		std::cout << "Cannot create window: " << this->name << std::endl;
#endif
		SDL_Quit();
		return;
	}


	SDL_GLContext context = SDL_GL_CreateContext(this->win);
	if (!context) {
#ifdef DEBUG
		std::cout << "Cannot load OpenGL [glad.h]" << std::endl;
#endif
		return;
	}

	SDL_GL_MakeCurrent(this->win, context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
#ifdef DEBUG
		std::cout << "gladLoadGLLoader failed!" << std::endl;
#endif
		return;
	}

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, this->dims.x, this->dims.y);
	this->camera = ZQcamera{ dvec3_t(0.0), vec3_t(0.0f, 0.0f, 0.0f), vec3_t(0.0f, 0.0f, 0.0f), vec2_t(90.0f, 90.0f * ((float)this->dims.x) / this->dims.y), this->dims };
	//glDisable(GL_CULL_FACE);

	this->load_shaders();
	this->load_models();
	this->load_textures();
	this->load_assets();

	this->level = new ZQlevel("resources/levels/test_level.zqlbin");

	this->last_time = 0.0f;
}

int PlayerCtrl::DEADZONE = 8000;
int PlayerCtrl::VIEW_MULTIPLAYER = 20;
int PlayerCtrl::VIEW_MULTIPLIER_KBD = 0.1f;

void ZQapp::main_loop() {

	GLuint fbo = 0,
		fboColorTex = 0;

	glGenTextures(1, &fboColorTex);
	glGenFramebuffers(1, &fbo);

	this->loop = true;
	while (this->loop) {

		SDL_PumpEvents();

		double cursorX, cursorY;

		//this->empty_CtrlQueues();

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) {
				this->loop = false;
				break;
			}

			if (ev.type == SDL_KEYDOWN) {
				if (ev.key.repeat == 0) {
					this->process_KBdDown(ev.key.keysym.scancode);
				}
				else if (ev.key.repeat >  0) {
					this->process_KBdHold(ev.key.keysym.scancode);
				}
			}

			else if (ev.type == SDL_KEYUP) {
				if (ev.key.keysym.scancode == SDL_SCANCODE_W) {
					//this->loop = false;
					break;
				}
			}
		}

		if (this->control_type == KBdMouse) {
			//glfwGetCursorPos(this->win, &cursorX, &cursorY);
			//glfwSetCursorPos(this->win, (double)(this->dims.x / 2), (double)(this->dims.y / 2));
			//cursor_input(&this->camera, this->dims, cursorX, cursorY);
			float pitch = 0.0f,
				yaw = 0.0f;

			int_t mouse_x = 0.0f, mouse_y = 0.0f;
			uint_t mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);

			int_t x_screen = mouse_x - (this->dims.x / 2),
				y_screen = mouse_y - (this->dims.y / 2);

			keyboard_view(&this->camera, x_screen, y_screen, PlayerCtrl::VIEW_MULTIPLIER_KBD);
		}
		else if (this->control_type == Controller) {
			SDL_Event e;

			float lx_axis = 0.0f;
			float ly_axis = 0.0f;
			float rx_axis = 0.0f;
			float ry_axis = 0.0f;
			const int DEADZONE = 8000;

			int16_t raw_lx = SDL_GameControllerGetAxis(this->joystick_left, SDL_CONTROLLER_AXIS_LEFTX);
			int16_t raw_ly = SDL_GameControllerGetAxis(this->joystick_left, SDL_CONTROLLER_AXIS_LEFTY);
			int16_t raw_rx = SDL_GameControllerGetAxis(this->joystick_left, SDL_CONTROLLER_AXIS_RIGHTX);
			int16_t raw_ry = SDL_GameControllerGetAxis(this->joystick_left, SDL_CONTROLLER_AXIS_RIGHTY);

			lx_axis = (abs(raw_lx) > PlayerCtrl::DEADZONE) ? (float)raw_lx / 32768.0f : 0.0f;
			ly_axis = (abs(raw_ly) > PlayerCtrl::DEADZONE) ? (float)raw_ly / 32768.0f : 0.0f;
			rx_axis = (abs(raw_rx) > PlayerCtrl::DEADZONE) ? (float)raw_rx / 32768.0f : 0.0f;
			ry_axis = (abs(raw_ry) > PlayerCtrl::DEADZONE) ? (float)raw_ry / 32768.0f : 0.0f;

			lx_axis *= this->last_time;
			ly_axis *= this->last_time;
			rx_axis *= PlayerCtrl::VIEW_MULTIPLAYER;
			ry_axis *= PlayerCtrl::VIEW_MULTIPLAYER;

			// 4. Update the Camera State
			controller_view(&this->camera, rx_axis, ry_axis);
			controller_move(&this->camera, lx_axis, ly_axis);

			/*while (SDL_PollEvent(&e)) {

				if (e.type == SDL_CONTROLLERAXISMOTION) {
					// Left Stick - Movement
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
						if (abs(e.caxis.value) > DEADZONE) lx_axis = (float)e.caxis.value / 32768;
						else lx_axis = 0.0f;
					}
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
						if (abs(e.caxis.value) > DEADZONE) ly_axis = (float)e.caxis.value / 32768;
						else ly_axis = 0.0f;
					}

					// Right Stick - Camera / Aiming
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
						if (abs(e.caxis.value) > DEADZONE) rx_axis = (float)e.caxis.value / 32768;
						else rx_axis = 0.0f;
					}
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
						if (abs(e.caxis.value) > DEADZONE) ry_axis = (float)e.caxis.value / 32768;
						else ry_axis = 0.0f;
					}
				}
				else {
					this->process_CtrlEvent(e);
				}
			}

			lx_axis *= this->last_time;
			ly_axis *= this->last_time;
			rx_axis *= this->last_time;
			ry_axis *= this->last_time;

			controller_view(&this->camera, rx_axis, ry_axis);*/
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
		ulong_t current = SDL_GetPerformanceCounter();
		ulong_t freq = SDL_GetPerformanceFrequency(); // Ticks per second
		if (freq == 0) {
			this->last_time = 0.0f;
		}
		else {
			this->last_time = (double)(current - this->prev_counter) / (double)freq;
			this->prev_counter = current;
		}

		glBindTexture(GL_TEXTURE_2D, fboColorTex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->dims.x, this->dims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorTex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#ifdef DEBUG
			std::cout << "Error: Framebuffer not made!" << std::endl;
#endif
		}
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(0, 0, this->dims.x, this->dims.y, 0, 0, this->dims.x, this->dims.y, GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

		SDL_GL_SwapWindow(this->win);

		//glfwPollEvents();
	}

	glDeleteTextures(1, &fboColorTex);
	glDeleteFramebuffers(1, &fbo);

	SDL_DestroyWindow(this->win);
	SDL_Quit();
}

void draw(ZQcamera* cam, ZQasset_static_instance* mod, ZQshader_program* prog) {
	ZQasset_static* asset = &Playerside::static_assets[mod->asset_idx];
#ifdef DEBUG
	std::cout << "Camera rot: { " << cam->rotation.x << ", " << cam->rotation.y << " }" << std::endl;
#endif
	d_ZQmodel* d_m;
	ZQmodel* modl;

	float dist = glm::length(mod->position - cam->position);

	if (dist < 100.0f) {
		d_m = &Playerside::models[asset->lod0_idx];
		modl = &Playerside::h_models[asset->lod0_idx];
	}
	else if (dist < 200.0f) {
		d_m = &Playerside::models[asset->lod1_idx];
		modl = &Playerside::h_models[asset->lod1_idx];
	}
	else if (dist < 300.0f) {
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

	float pitch = glm::radians(cam->rotation.x);
	float yaw = glm::radians(cam->rotation.y);

	vec3_t forward = vec3_t(
						cosf(pitch) * sinf(yaw),
						sinf(pitch),
						-cosf(pitch) * cosf(yaw));
	forward = glm::normalize(forward);

	vec3_t cam_pos = vec3_t(cam->position);
	vec3_t target_pos = cam_pos + forward;

	vec3_t up = vec3_t(0.0f, 1.0f, 0.0f);

	mat4_t geo = glm::lookAt(cam_pos, target_pos, up);
	
	/*model = glm::translate(model, vec3_t(mod->position));
	model = glm::rotate(model, glm::radians(mod->rotation.x), vec3_t(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(mod->rotation.y), vec3_t(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(mod->rotation.z), vec3_t(0.0f, 0.0f, 1.0f));*/

	/*mat4_t geo = mat4_t(1.0f);
	geo = glm::translate(geo, -vec3_t(cam->position));
	geo = glm::rotate(geo, glm::radians(cam->rotation.x), vec3_t(1.0f, 0.0f, 0.0f));
	geo = glm::rotate(geo, glm::radians(cam->rotation.y), vec3_t(0.0f, 1.0f, 0.0f));
	//geo = glm::rotate(geo, glm::radians(0.0f), vec3_t(0.0f, 0.0f, 1.0f));*/

	//geo = glm::inverse(geo);

	mat4_t persp = glm::perspective(glm::radians(cam->fov.x), (float)cam->dims.x / cam->dims.y, 0.01f, 10000.0f);

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