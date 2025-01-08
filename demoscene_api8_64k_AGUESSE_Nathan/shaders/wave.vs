#version 330

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 vsiNormal;
layout (location = 2) in vec2 vsiTexCoord;

#define NMB_SIN 5

out VertexData {
  float height;
  vec3 normal;
  vec4 mpos;
} vs_out;

uniform mat4 proj, model, view;
uniform float time;
// number of sines used to creates the waves
uniform int nmb_sin;
// get the cube z position to go height distortion
uniform float cubePosZ;

// every parameters of all the sines, generating from cpu side
uniform float maxAmp;
uniform vec2 directions[NMB_SIN];
uniform float amps[NMB_SIN];
uniform float freqs[NMB_SIN];
uniform float speeds[NMB_SIN];

uniform int circular;

// calculate the sine with its amplitude, freq and speed, and also knowing which direction is the wave
float sine(vec2 pos, float t, vec2 dir, float amp, float freq, float speed) {
  vec2 dirN = normalize(dir);
  float x = 0.0;
  if (circular == 0)
    x = pos.x * dirN.x + pos.y * dirN.y;
  if ((circular == 1) || ((time - length((model * vec4(vsiPosition, 1.0)).xz)/10.0f) > 108.0f)) {
    x = length(pos);
    t /= -1.0;
  }
  return amp * sin(x * freq + t * (speed*freq));
}

float cosine(vec2 pos, float t, vec2 dir, float amp, float freq, float speed) {
  vec2 dirN = normalize(dir);
  float x = pos.x * dirN.x + pos.y * dirN.y;
  if (circular == 0)
    x = pos.x * dirN.x + pos.y * dirN.y;
  if (circular == 1)
    x = length(pos);
  return amp * cos(x * freq + t * (speed*freq));
}

// calculate all sines, to get the sum
float sum_of_sines(vec2 pos, float t) {
  float sum = 0.0, sumX = 0.0, sumY = 0.0;
  for (int i = 0; i < nmb_sin; i++) {
    vec2 dirN = normalize(directions[i]);

    // sum of sine for vertex height
    float s = sine(pos, t, directions[i], amps[i], freqs[i], speeds[i]);
    sum += s;

    // derivative for vertex normal
    float dx = dirN.x * freqs[i] * cosine(pos, t, directions[i], amps[i], freqs[i], speeds[i]);
    float dy = dirN.y * freqs[i] * cosine(pos, t, directions[i], amps[i], freqs[i], speeds[i]);
    sumX += dx;
    sumY += dy;
  }
  //vec3 bino = vec3(1.0, sumX, 0.0);
  //vec3 tang = vec3(0.0, sumY, 1.0);
  //vs_out.normal = normalize(cross(bino, tang));
  vs_out.normal = normalize(vec3(-sumX, 1.0, -sumY));
  if ((time - length((model * vec4(vsiPosition, 1.0)).xz)/10.0f) > 108.0f) {
    sum = sine(pos, t, vec2(0.981103, -0.136386), 0.689502/25.0, 151.260254, 0.070094) +
          sine(pos, t, vec2(-0.554842, 0.939381), 0.455034/25.0, 23.521606, 0.220191) +
          sine(pos, t, vec2(-0.801930, 0.373775), 0.404072/25.0, 12.625463, 0.228090) +
          sine(pos, t, vec2(-0.126957, -0.896222), 0.754823/25.0, 43.392372, 0.170792) +
          sine(pos, t, vec2(-0.130497, 0.520540), 0.196569/25.0, 18.101688, 0.121263);
  }
  return sum;
}


void main() {
  // get y of the vertex, with the sum of sines
  vec3 scaledPos = vec3(vsiPosition.x*1.5, vsiPosition.y, vsiPosition.z);
  float newY = sum_of_sines(scaledPos.xz, time);
  vec3 pos = vec3(scaledPos.x, scaledPos.y+newY, scaledPos.z);
  // clamp the height obtained, between 0 and 1 for coloring purpose
  vs_out.height = (newY/maxAmp+1.0)/2.0;
  if ((time - length((model * vec4(vsiPosition, 1.0)).xz)/10.0f) > 108.0f) {
    vs_out.height = (newY/0.1+1.0)/2.0;
  }
  vs_out.mpos = model * vec4(pos, 1.0);
  if (vs_out.mpos.z < cubePosZ) {
    vs_out.mpos.y += (cubePosZ - vs_out.mpos.z);
  }
  gl_Position = proj * view * vs_out.mpos;
}