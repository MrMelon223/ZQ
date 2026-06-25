#ifndef ZQAPP_H
#define ZQAPP_H

#include "ZQlevel.h"

#include "../cuda/include/ZQrender.cuh"

enum ZQcontrol {
	KBdMouse,
	Controller
};

typedef struct {
	int_t key;
	int_t scancode;
	int_t action;
	int_t mode;
} KBdUse;

namespace PlayerCtrl {
	extern std::queue<KBdUse> keyboard_queue;
	extern int DEADZONE;
	extern int VIEW_MULTIPLAYER;
}

//static void key_callback(GLFWwindow*, int, int, int, int);

class ZQapp {
protected:
	ZQcompute* compute;

	std::string name;

	dim_t dims;

	bool fullscreen;
	short_t monitor;

	SDL_Window* win;

	bool loop;

	ZQcamera camera;

	void process_KBdUse(KBdUse);

	void process_CtrlEvent(SDL_Event);

	void empty_CtrlQueues();

	void load_shaders();
	void load_models();
	void load_assets();
	void load_textures();

	ZQlevel* level;

	std::vector<ZQasset_static_instance> compute_visibility(ZQcamera*);

	float last_time;
	ulong_t prev_counter;

	ZQcontrol control_type;
	SDL_GameController* joystick_left;
	//SDL_Joystick* joystick_right;
public:
	ZQapp(ZQcompute*, std::string, dim_t, bool, short_t);

	void init();

	void main_loop();
};


void draw(ZQcamera*, ZQasset_static_instance*, ZQshader_program*);

#endif
