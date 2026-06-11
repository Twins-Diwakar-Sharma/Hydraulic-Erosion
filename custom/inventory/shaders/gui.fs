#version 400

out vec4 fragColor;

in vec2 fragTex;

uniform sampler2D albedo;

void main()
{
  vec4 col = texture(albedo, fragTex);
  fragColor = col;
}
