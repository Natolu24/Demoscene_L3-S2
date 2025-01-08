#version 330
in vec2 tcoord;
out vec4 fragColor;

uniform sampler2D tex;
uniform float time;

void main() {
  float offsetY = 0.01 * sin(tcoord.x * 100 + time * (0.01*100));
  float waveColorHeight = ((offsetY/0.01)+1.0)/2.0;
  vec4 c = texture(tex, vec2(tcoord.x, tcoord.y + offsetY));
  if(length(c.rgb) > 0.01)
    fragColor = vec4(waveColorHeight+0.2, 1.0, 1.0, 1.0);
  else
    discard;
}
