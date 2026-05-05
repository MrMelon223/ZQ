#version 300 es
precision highp float;

uniform mat4 matGeo;
uniform mat4 matVP;
uniform mat4 matCam;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tnormal;

out vec4 color;
out vec2 texcoord;

void main() {

   gl_Position = matVP * matCam * matGeo  * vec4(pos, 1.0);
   texcoord = uv;

}
