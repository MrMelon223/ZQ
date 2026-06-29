#version 300 es
precision highp float;

in vec4 color;
in vec2 texcoord;

uniform sampler2D color_map;

uniform float min_distance;
uniform float max_distance;

out float depth_map;

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0; 
    return (2.0 * min_distance * max_distance) / (max_distance + min_distance - z * (max_distance - min_distance));  
}

void main() {
   depth_map = linearize_depth(gl_FragCoord.z) / max_distance;
}