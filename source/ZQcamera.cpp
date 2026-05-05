	// ZQcamera.cpp
#include "../include/ZQcamera.h"

void cursor_input(ZQcamera* cam, dim_t dims, double x, double y) {
	if (x != (double)cam->dims.x * 0.5 && y != (double)cam->dims.y * 0.5 && (x > 0.0f && x < (double)dims.x && y > 0.0f && y < (double)dims.y)) {
		float rX = ((float)x - (float)(dims.x / 2) / dims.x) * cam->fov.x * 0.5f;
		float rY = ((float)y - (float)(dims.y / 2) / dims.y) * cam->fov.y * 0.5f;
		
		cam->rotation.x += rX / 100000.0f;
		cam->rotation.y += rY / 100000.0f;
		cam->rotation.z = 0.0f;

		if (cam->rotation.x < 0.0f) {
			cam->rotation.x = -fmodf(cam->rotation.x, 360.0f);
		}
		else {
			cam->rotation.x = fmodf(cam->rotation.x, 360.0f);
		}

		if (cam->rotation.y < 0.0f) {
			cam->rotation.y = -fmodf(cam->rotation.y, 360.0f);
		}
		else {
			cam->rotation.y = fmodf(cam->rotation.y, 360.0f);
		}
	}
}

float CTRL_DEADZONE_MOVE = 0.075f;
float CTRL_MOVE_MULTIPLIER = 0.075f;

void controller_input(ZQcamera* camera, dim_t dims, GLFWgamepadstate& state) {
	// Access axis values
	float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
	float leftY = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
	float rightX = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
	float rightY = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];

	// Trigger values (often range from -1.0 to 1.0)
	float leftTrigger = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
	float rightTrigger = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];

	if (rightY > CTRL_DEADZONE_MOVE || rightY < -CTRL_DEADZONE_MOVE) {
		camera->rotation.x += rightY * CTRL_MOVE_MULTIPLIER;
	}
	if (rightX > CTRL_DEADZONE_MOVE || rightX < -CTRL_DEADZONE_MOVE) {
		camera->rotation.y -= rightX * CTRL_MOVE_MULTIPLIER;
	}

	if (camera->rotation.y < 0.0f) {
		camera->rotation.y = -fmodf(camera->rotation.y, 360.0f);
	}
	else {
		camera->rotation.y = fmodf(camera->rotation.y, 360.0f);
	}

	if (camera->rotation.x < -70.0f) {
		camera->rotation.x = fmodf(camera->rotation.x, 360.0f);
		camera->rotation.x = -70.0f;
	}
	else if (camera->rotation.x > 70.0f) {
		camera->rotation.x = fmodf(camera->rotation.x, 360.0f);
		camera->rotation.x = 70.0f;
	}

	if (leftY > CTRL_DEADZONE_MOVE || leftY < -CTRL_DEADZONE_MOVE) {
		camera->position += glm::radians(camera->rotation) * leftY * CTRL_MOVE_MULTIPLIER;
	}
	if (leftX > CTRL_DEADZONE_MOVE || leftX < -CTRL_DEADZONE_MOVE) {
		camera->position += glm::cross(glm::radians(camera->rotation), glm::radians(camera->rotation)) * leftX * CTRL_MOVE_MULTIPLIER;
	}
}