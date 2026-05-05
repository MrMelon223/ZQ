#version 300 es
precision highp float;

in vec4 color;
in vec2 texcoord;
uniform sampler2D color_map;

out vec4 FragColor;

void main() {
   FragColor = texture(color_map, texcoord);
   //FragColor = vec4(texcoord, 0.0, 1.0);
}