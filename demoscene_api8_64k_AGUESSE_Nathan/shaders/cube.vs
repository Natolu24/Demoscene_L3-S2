#version 330

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 vsiNormal;
layout (location = 2) in vec2 vsiTexCoord;

out vec4 mpos;

uniform mat4 proj, model, view;

void main() {
  mpos = model * vec4(vsiPosition, 1.0);
  gl_Position = proj * view * mpos;
}