#version 330


struct DirectionalLight
{
	vec3 dir;
	vec3 col;
};
uniform DirectionalLight sun;

struct Camera 
{
	vec4 spin;
	vec3 pos;
};

const float threshold = 1.20;
const float noiseSpan = 64;

// random value between -1 and 1
vec2 randomVec2(vec2 st)
{
    st = vec2( dot(st,vec2(0.650,-0.420)),
              dot(st,vec2(-0.290,0.260)) );
    return -1.0 + 2.0*fract(sin(st)*12024.561);
}

// this needs a global const variable : noiseSpan 
float perlinNoise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

   // vec2 u = f*f*(3.0-2.0*f);
    vec2 u = f*f*f*(f*(f*6.0-15.0)+10.0);
    return mix( mix( dot( randomVec2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( randomVec2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( randomVec2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( randomVec2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

float fbm (vec2 p) 
{
  
    // Initial values
    float value = 0.0;
    float amplitude = 0.5;

    // Loop of octaves
    int OCTAVES = 3;
    for (int i = 0; i < OCTAVES; i++) 
    {

        value += amplitude * (perlinNoise(p));
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float fractal(vec2 p, int riged, float prevH)
{
   float val = perlinNoise(p); 
  if(riged > 0)
    val = 1.0 - abs(val);
  val = prevH * val;
  return val;
}

uniform sampler2D heightmap;
uniform int scale;
uniform vec2 config;

/*
float getProceduralHeight(float x, float z)
{

  float res = 1024.0;
  vec2 pos = vec2(x,z);
  //pos = pos/config.y;
  if(pos.x <= 0.0 || pos.x >= res || pos.y <= 0.0 || pos.y >= res)
  {
      return -64.0;
  }

  pos = pos/res;

  float h = texture(heightmap, pos).r;

  h = 2*h - 1.0;
  h = h*128;

  return h;
}
*/

float getProceduralHeight(float x, float z)
{
  float res = 1024;
  vec2 pos = vec2(x,z);
  //pos = pos/config.y;
  float h = 0.0; // in fractal function h gets multiplied, therefore 1 not 0
  if(pos.x >= -res/2 && pos.x <= res/2 && pos.y >= -res/2 && pos.y <= res/2)
  {
    vec2 texPos = ( pos + vec2(res/2))/res;
    texPos.y = -texPos.y;
    vec3 hCol = texture(heightmap, texPos).rgb;
    //h = (hCol.r + hCol.g + hCol.b)/3.0;
    h = hCol.r;
    h = 2*h - 1.0;
  }
  return h * 100.0;
}

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

layout (location = 0) in vec2 pos;

uniform mat4 projection;
uniform Camera cam;
uniform vec2 worldPos;
uniform vec2 center;
uniform vec2 stripScale;
//uniform int scale; shifted above
//uniform sampler2D heightmap; shifted above
//uniform vec2 config; shifted above

out vec3 fragWorldPos;
out vec2 fragRadialPos;


float interpolatedHeight(float x, float z)
{
  float blocksWidth = config.x;
  float spacing = config.y;

  float radius = blocksWidth/2 * spacing * scale;
  float gap = spacing * scale;
  float R = radius - 1*gap;
  float transWidth = 4*gap;



  float actualH = getProceduralHeight(x, z); 
 
  vec2 relCamPos = abs(vec2(x,z) - cam.pos.xz);


  vec2 alpha;
  relCamPos = relCamPos - (R - transWidth)*vec2(1,1);
  alpha = relCamPos/transWidth;
  alpha.x = clamp(relCamPos.x, 0, 1);
  alpha.y = clamp(relCamPos.y, 0, 1);
  float maxAlpha = max(alpha.x, alpha.y);  
  maxAlpha = smoothstep(0,1,maxAlpha);
   

  
    vec2 radialPos = abs(vec2(x,z) - center);
  //if(radialPos.x >= interRatio*radius || radialPos.y >= interRatio*radius)
  //{

    radialPos += vec2(gap, gap);
    radialPos = (mod(radialPos, 2*gap)); 
    float coarseH = actualH;
    float h0 = getProceduralHeight(x - radialPos.x, z + radialPos.y); 
    float h1 = getProceduralHeight(x + radialPos.x, z - radialPos.y); 
    coarseH = 0.5 * (h0 + h1);
    actualH = (1.0 - maxAlpha)*actualH + maxAlpha*coarseH;

  //}

  return actualH;
}


vec3 getNormal(vec3 pos)
{
    float delta = 1.0;
    float dfdz = (interpolatedHeight(pos.x, pos.z+delta) - pos.y)/delta;
    float dfdx = (interpolatedHeight(pos.x+delta, pos.z) - pos.y)/delta;
    
    vec3 ans = normalize(vec3(-dfdx, 1.0, -dfdz));
    return ans;
}

#define n_cascades 4
uniform Camera cascSunCam[n_cascades];
uniform mat4 cascOrtho[n_cascades];

out vec4 fragLightProjPos[n_cascades];
out float fragDepth;

//out vec3 fragNorm;

void main()
{
  vec2 objPos = stripScale * vec2(pos);
  objPos = scale * objPos;
  vec3 position = vec3(objPos.x + worldPos.x, 0, objPos.y + worldPos.y);
  position.y = interpolatedHeight(position.x, position.z);
  fragWorldPos = position;
  vec3 viewPos = position - cam.pos;
  viewPos = rotateWRTCamera(viewPos, cam);
  gl_Position = projection * vec4(viewPos, 1);

  vec2 radialPos = abs(vec2(position.x,position.z) - center);
  fragRadialPos = radialPos/scale;


/* 
  fragDepth = viewPos.z;
  for(int i=0; i<n_cascades; i++)
  {
    vec3 lightViewPos = position - cascSunCam[i].pos;
    lightViewPos = rotateWRTCamera(lightViewPos, cascSunCam[i]);
    fragLightProjPos[i] = cascOrtho[i] * vec4(lightViewPos, 1);
  }
*/
 

}
