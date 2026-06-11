#version 400

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex;

out vec2 fragTex;

uniform vec3 scalePos;

void main()
{
  vec2 worldPos = scalePos.x * pos + scalePos.yz;
  gl_Position = vec4(worldPos, 0, 1);

  fragTex = tex;
}
