	// main.cpp
#define SDL_MAIN_HANDLED
#include "../include/ZQapp.h"

int main(int argc, char* argv[]) {

	ZQcompute* c = new ZQcompute();

	ZQapp* app = new ZQapp(c, "ZQ-debug-0.0", dim_t(1280, 720), false, 0);

	app->init();

	app->main_loop();

	return 0;
}

