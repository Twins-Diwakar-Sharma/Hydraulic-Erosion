#version 400

in vec2 fragTex;

out vec4 outColor;

uniform sampler2D albedo;

void main()
{
  vec4 color = texture(albedo, fragTex);

	if(color.a < 0.1)
		discard;

	outColor = vec4(color.rgb,1.0);
}
