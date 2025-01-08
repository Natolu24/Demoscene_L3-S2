#version 330
out vec4 fragColor;

in vec4 mpos;

uniform float time;
uniform float realTime;

void main(void) {
  //fragColor = vec4(fs_in.normal, 1.0);
  //fragColor = vec4(float(int(mpos.x)%100)/100, 0.0, 0.0, 1.0);
  //fragColor = vec4(1.0, 1.0, fs_in.height, 1.0);
  //fragColor = color;
  if ((realTime - length(mpos.xy)/10.0f) < 69.0f) {
    fragColor = vec4(1.0, 1.0, sin(length(mpos.xy)+time*-10) + 0.2, 1.0);
  }
  else {
    fragColor = vec4(sin(length(mpos.xy)+time*-10), sin(length(mpos.xy)+time*-10), sin(length(mpos.xy)+time*-10), 1.0);
  }
}
