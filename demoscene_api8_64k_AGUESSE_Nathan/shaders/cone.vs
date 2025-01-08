#version 330

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 vsiNormal;
layout (location = 2) in vec2 vsiTexCoord;

#define NMB_SIN 5

out float peak;

uniform mat4 proj, model, view;
uniform float time;

void main() {
  vec4 mpos = model * vec4(vsiPosition, 1.0);
  if (vsiPosition.y > 0.0) {
    peak = 1.0;
  }
  else {
    peak = 0.0;
  }
  gl_Position = proj * view * mpos;
}