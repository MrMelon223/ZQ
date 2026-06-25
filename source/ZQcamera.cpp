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

void controller_input(ZQcamera* camera, dim_t dims) {
	// Access axis values
/*	float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
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
	}*/
}

void controller_view(ZQcamera* camera, float x, float y) {
	if (y > CTRL_DEADZONE_MOVE || y < -CTRL_DEADZONE_MOVE) {
		camera->rotation.x -= y * CTRL_MOVE_MULTIPLIER;
	}
	if (x > CTRL_DEADZONE_MOVE || x < -CTRL_DEADZONE_MOVE) {
		camera->rotation.y += x * CTRL_MOVE_MULTIPLIER;
	}

	if (camera->rotation.y < 0.0f) {
		camera->rotation.y = 360.0f;
	}

	camera->rotation.x = fmodf(camera->rotation.x, 360.0f);
	camera->rotation.x = fmodf(camera->rotation.x, 360.0f);
	if (camera->rotation.x < -70.0f) {
		camera->rotation.x = -70.0f;
	}
	else if (camera->rotation.x > 70.0f) {
		camera->rotation.x = 70.0f;
	}
#ifdef DEBUG
	std::cout << "Adjusting controller view!" << std::endl;
#endif
}

void controller_move(ZQcamera* camera, float x, float y) {
	const float MOVE_SPEED = 50.0f;

	// 2. Convert degrees to radians
	float pitch = glm::radians(camera->rotation.x);
	float yaw = glm::radians(camera->rotation.y);

	// 3. Calculate where the camera is looking overall
	glm::vec3 forward;
	forward.x = cos(pitch) * sin(yaw);
	forward.y = sin(pitch);
	forward.z = -cos(pitch) * cos(yaw);
	forward = glm::normalize(forward);

	// 4. Project onto the horizontal floor plane (X and Z only)
	glm::vec3 ground_forward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));

	// 5. Find the perpendicular "Right" vector using a cross product
	glm::vec3 ground_right = glm::normalize(glm::cross(ground_forward, glm::vec3(0.0f, 1.0f, 0.0f)));

	// 6. Update the physical position coordinates
	// ly_axis is negative when pushing forward on standard sticks, so we invert it
	camera->position += dvec3_t(ground_forward) * (double)(-y * MOVE_SPEED);
	camera->position += dvec3_t(ground_right) * (double)(x * MOVE_SPEED);
#ifdef DEBUG
	std::cout << "Adjusting controller position" << std::endl;
#endif
}