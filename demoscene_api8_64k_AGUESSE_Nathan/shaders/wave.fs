#version 330
out vec4 fragColor;

in VertexData {
  float height;
  vec3 normal;
  vec4 mpos;
} fs_in;

uniform float time;
uniform int color_choose;

vec3 Lp = vec3(0.0, -10.0, 10.0);

void main(void) {
  Lp.y += cos(time)*10;
  vec3 Ld = normalize(vec3(0.0, -1.0, 0.0));
  float intensiteDeLumiereDiffuse = clamp(dot(fs_in.normal, -Ld), 0.0, 1.0);
  vec4 color = intensiteDeLumiereDiffuse * vec4(0.0, 1.0, 1.0, 1.0);
  if (color_choose == 0)
    fragColor = vec4(fs_in.height+0.2, 1.0, 1.0, 1.0);
  if (color_choose == 1)
    fragColor = vec4(fs_in.height+0.2, 1.0, fs_in.height+0.2, 1.0);
  if (color_choose == 2)
    fragColor = vec4(1.0, 1.0, fs_in.height+0.2, 1.0);
  if (color_choose == 3)
    fragColor = vec4(1.0, fs_in.height+0.2, 1.0, 1.0);
  if (color_choose == 4)
    fragColor = vec4(fs_in.height+0.2, fs_in.height+0.2, 1.0, 1.0);
  if (color_choose == 5)
    fragColor = vec4(1.0, (fs_in.height+1.0)/2.0+0.2, fs_in.height+0.2, 1.0);
  if (color_choose == 6)
    fragColor = vec4(1.0, fs_in.height+0.2, fs_in.height+0.2, 1.0);
  if (color_choose == 7)
    fragColor = vec4(fs_in.height, fs_in.height, fs_in.height, 1.0);
  //fragColor = vec4(fs_in.normal, 1.0);
  //fragColor = vec4(float(int(fs_in.mpos.x)%100)/100, 0.0, 0.0, 1.0);
  //fragColor = vec4(fs_in.height, fs_in.height, 1.0, 1.0);
  //fragColor = color;
}
