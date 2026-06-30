#version 300 es
precision highp float;

uniform mat4 matGeo;
uniform mat4 matVP;
uniform mat4 matCam;

layout (location = 0) in vec3 pos;

void main() {
	gl_Position = matVP * matCam * matGeo * vec4(pos, 1.0);
}