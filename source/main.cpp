	// main.cpp
#include "../include/ZQapp.h"

int main() {

	ZQcompute* c = new ZQcompute();

	ZQapp* app = new ZQapp(c, "ZQ-debug-0.0", dim_t(640, 480), false, 0);

	app->init();

	app->main_loop();

	return 0;
}