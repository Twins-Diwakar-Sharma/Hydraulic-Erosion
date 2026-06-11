#version 400

out vec4 fragColor;

in vec2 fragTex;

uniform sampler2D albedo;
uniform sampler2D depth;
uniform vec2 screenDim;

void main()
{

  vec4 col = texture(albedo, fragTex);
  vec4 depthCol = texture(depth, fragTex);

 //col.rgb = col.rgb / (col.rgb + vec3(1.0));


 vec3 colorHDR = pow(col.rgb, vec3(1.0/2.2));

fragColor = vec4(colorHDR,col.a);
}
