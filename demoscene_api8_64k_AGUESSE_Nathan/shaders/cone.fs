#version 330
out vec4 fragColor;

in float peak;

uniform float time;
uniform int color;

void main(void) {
  if (color == 0)
    fragColor = vec4(peak, 1.0, 1.0, 1.0);
  if (color == 1)
    fragColor = vec4(peak, 1.0, peak, 1.0);
  if (color == 2)
    fragColor = vec4(1.0, 1.0, peak, 1.0);
  if (color == 3)
    fragColor = vec4(1.0, peak, 1.0, 1.0);
  if (color == 4)
    fragColor = vec4(peak, peak, 1.0, 1.0);
  if (color == 5)
    fragColor = vec4(1.0, (peak+1.0)/2.0, peak, 1.0);
  if (color == 6)
    fragColor = vec4(1.0, peak, peak, 1.0);
}
