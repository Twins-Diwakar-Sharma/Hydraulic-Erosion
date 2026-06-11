#version 330


struct DirectionalLight
{
	vec3 dir;
	vec3 col;
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
  pos = pos;
  //pos = pos/config.y;
  if(pos.x <= 0.0 || pos.x >= res || pos.y <= 0.0 || pos.y >= res)
  {
      return -64.0;
  }

  pos = pos/res;

  float h = texture(heightmap, pos).r;
  //h = 2*h - 1.0;
  //h = h*128 + 50;
  
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


uniform sampler2D normalmap;

/*
vec3 getNormal(vec3 pos)
{

  float res = 1024.0;
  //pos = pos/config.y;
  if(pos.x <= 0.0 || pos.x >= res || pos.z <= 0.0 || pos.z >= res)
  {
      return vec3(0,1,0);
  }

  pos.xz = pos.xz/res;


  vec3 norm = texture(normalmap, pos.xz).rgb; 
  norm = 2.0 * norm - 1.0;
  
  return normalize(norm);
}
*/

vec3 getNormal(vec3 pos) 
{
  //float delta = 0.01;
  float gridGap = 4; // smoothing
  float delta = gridGap/2;
  float dfdz = getProceduralHeight(pos.x, pos.z+delta) - getProceduralHeight(pos.x, pos.z-delta);
  float dfdx = getProceduralHeight(pos.x+delta, pos.z) - getProceduralHeight(pos.x-delta, pos.z);
  return normalize(vec3(-dfdx/delta, gridGap, -dfdz/delta));
}

in vec3 fragWorldPos;
in vec2 fragRadialPos;
out vec4 outColor;

uniform DirectionalLight sun;
uniform int debugCol;

#define n_cascades 4
uniform sampler2D cascShadowmap[n_cascades];
uniform float cascFarPlane[n_cascades];
in vec4 fragLightProjPos[n_cascades];
in float fragDepth;
//in vec3 fragNorm;


void main()
{
  float currH = getProceduralHeight(fragWorldPos.x, fragWorldPos.z);
  vec3 position = vec3(fragWorldPos.x, currH, fragWorldPos.z);
  vec3 fragNorm = getNormal(position); 
  
	vec3 toLight = normalize(-1*sun.dir);	
  int cascIdx = n_cascades-1;
/*
  for(int i=0; i<n_cascades; i++)
  {
    if(abs(fragDepth) <= cascFarPlane[i])
    {
      cascIdx = i;
      break;
    }
  }
  //cascIdx = 2;
  vec3 projCoords = fragLightProjPos[cascIdx].xyz / fragLightProjPos[cascIdx].w;
  projCoords = projCoords * 0.5 + 0.5;
  float bias = max(0.05 * (1.0 - dot(fragNorm, toLight)), 0.005);
  float shadow = 0.0;
  float pcfDepth = texture(cascShadowmap[cascIdx], projCoords.xy).r;
  shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
  if(projCoords.z > 1.0)
    shadow = 0.0;
*/
  float shadow = 0;

  float stepScale = 1;
  float step = dot(-sun.dir, vec3(0,1,0)) * stepScale + 2;
  step = 5; 
  int num_steps = 50;
  shadow = 0;
  float bias = max(0.05 * (1.0 - dot(fragNorm, toLight)), 0.005);
  float posDistField = 10000;
  for(int i=0; i<num_steps; i++)
  {
    vec3 toSun = -sun.dir;
    vec3 checkPos = position + (i+1)*step*toSun;
    float height = getProceduralHeight(checkPos.x, checkPos.z);    
    float distf = max(checkPos.y - height, 0.0);
    posDistField = min(posDistField, distf);
    if(checkPos.y + bias < height)
    {
      shadow = 1.0;
      break;
    }
  }

  float thresh = 1.5;
  if(shadow == 0 && posDistField <= thresh)
  {
    posDistField = posDistField/thresh;
    shadow = 1.0 - posDistField;
  }

  //float occ = doGorgeousOcclusion( pos, nor );
  //float sha = doGreatSoftShadow( pos, sunDir );
  float dotsun = clamp( dot( fragNorm, -sun.dir), 0.0, 1.0 );
  float dotsky = clamp( 0.5 + 0.5*fragNorm.y, 0.0, 1.0 );
  float ind = clamp( dot( fragNorm, normalize(-sun.dir*vec3(-1.0,0.0,-1.0)) ), 0.0, 1.0 );
  float sha = smoothstep(0.0, 1.0, (1.0 - shadow));
  float occ = 1.0;

  // compute lighting
  // realistic sun colvec3(1.64,1.27,0.99)*
  vec3 lin  = dotsun*sun.col*pow(vec3(sha),vec3(1.0,1.2,1.5));
          lin += dotsky*vec3(0.16,0.20,0.28)*occ;
          lin += ind*vec3(0.40,0.28,0.20)*occ;


  vec3 material = vec3(0.12,0.14,0.18);
  
  outColor.a = 1;
  outColor.rgb = material*lin;

  //outColor.rgb = vec3(texture(heightmap,position.xz/256).r);

  //outColor.rgb = pow(outColor.rgb, vec3(1.0/2.2));
}
