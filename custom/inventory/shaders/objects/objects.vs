#version 400

struct Camera 
{
	vec4 spin;
	vec3 pos;
};

vec4 quatRotate(inout vec4 action, inout vec4 victim)
{
	float ar = action.w;	float br = victim.w;
	vec3 av = action.xyz;	vec3 bv = victim.xyz;
	return vec4(ar*bv + br*av + cross(av,bv), ar*br - dot(av,bv));
}

vec3 rotateWRTCamera(vec3 viewPos, in Camera cam)
{

	vec4 spin = cam.spin;
	vec4 quatView = vec4(viewPos,0);
	vec4 spinQuat = vec4(-spin.xyz, spin.w);
	vec4 spinQuatInv = vec4(spin);

	quatView = quatRotate(quatView, spinQuatInv);
	quatView = quatRotate(spinQuat, quatView);
	
	return quatView.xyz;
}


layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 nor;

out vec2 fragTex;
out vec3 fragNor;

uniform mat4 projection;
uniform mat4 transform;
uniform Camera cam;

void main()
{
  vec4 worldPos = transform * vec4(pos,1);
  vec3 viewPos = worldPos.xyz - cam.pos;
  viewPos = rotateWRTCamera(viewPos, cam);
  gl_Position = projection * vec4(viewPos,1);  
  fragTex = vec2(tex);

  fragNor = normalize((transform * vec4(nor,0.0)).xyz);
}
