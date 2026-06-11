#include "GeoTerrain.h"

GeoTerrain::GeoTerrain()
{}

GeoTerrain::~GeoTerrain()
{}

void GeoTerrain::initialize(int blocksWidth, float spacing)
{
  // ensure perfect 2^k - 1 grid lines or we can say 2^k - 2 blocksWidth
  int gridLines = blocksWidth-1;
  int twoPower = 1;
  while(gridLines != 0)
  {
    gridLines = gridLines >> 1;
    twoPower = twoPower << 1;
  }
  std::cout << "GeoTerrain: resetting " << blocksWidth << " to nearest 2^k - 2 : " << twoPower - 2 << std::endl;
  blocksWidth = twoPower - 2;
  this->blocksWidth = blocksWidth;
  this->spacing = spacing;
  gridFull.create(GRID_FULL, blocksWidth, spacing);
  gridHollow.create(GRID_HOLLOW, blocksWidth, spacing);
  strip.create(STRIP, blocksWidth, spacing);
  stripAlt.create(STRIP_ALT, blocksWidth, spacing);
  
  layers[0].step = Vec2(0.5f,0.5f);
  layers[0].scale = 1;
  layers[0].pos = layers[0].pos + (spacing * layers[0].scale * Vec2(-1,-1));
  layers[0].stripPos = Vec2(0,0); // absent
  for(int i=1; i<n_layers; i++)
  {
    layers[i].step = Vec2(0.5f,0.5f);
    layers[i].scale = 2 * layers[i-1].scale; 
    layers[i].pos = layers[i].pos + (spacing * layers[i].scale * Vec2(-1,-1));
    float scl = (blocksWidth/2  + 1)* layers[i-1].scale;
    layers[i].stripPos = layers[i-1].pos + (scl * spacing * -2 * layers[i-1].step);
  }
}

void GeoTerrain::updateLayer(int i, bool changeX, bool changeZ)
{
  if(i >= n_layers)
    return;

  // -1.5*  -0.5  +0.5  +1.5*
  if(changeX)
  {
    changeX = false;
    if(layers[i].step[0] > 0) // moving forward in x
    {
      layers[i].pos[0] += 2 * layers[i].scale * spacing;
      if(layers[i].step[0] > 1)
      {
        layers[i].step[0] -= 2.0;
        if(i+1 < n_layers)
        {
          layers[i+1].step[0]++;
          changeX = true;
        }
      }
    } 
    else if(layers[i].step[0] < 0) // moving backwards in x
    {
      layers[i].pos[0] -= 2 * layers[i].scale * spacing;
      if(layers[i].step[0] < -1)
      {
        layers[i].step[0] += 2.0;
        if(i+1 < n_layers)
        {
          layers[i+1].step[0]--;
          changeX = true;
        }
      }
    }
  }
  
  if(changeZ)
  {
    changeZ = false;
    if(layers[i].step[1] > 0) // moving forward in z
    {
      layers[i].pos[1] += 2 * layers[i].scale * spacing;
      if(layers[i].step[1] > 1)
      {
        layers[i].step[1] -= 2.0;
        if(i+1 < n_layers)
        {
          layers[i+1].step[1]++;
          changeZ = true;
        }
      }
    } 
    else if(layers[i].step[1] < 0) // moving backwards in z
    {
      layers[i].pos[1] -= 2 * layers[i].scale * spacing;
      if(layers[i].step[1] < -1)
      {
        layers[i].step[1] += 2.0;
        if(i+1 < n_layers)
        {
          layers[i+1].step[1]--;
          changeZ = true;
        }
      }
    }
  }

  if(i+1 < n_layers)
  {
    float scl = (blocksWidth/2  + 1)* layers[i].scale;
    layers[i+1].stripPos = layers[i].pos + (scl * spacing * -2 * layers[i].step);
  }

  if(changeX || changeZ)
  {
    updateLayer(i+1, changeX, changeZ);
  }

}

void GeoTerrain::update(Vec3& camPos)
{
  
  Vec2 delta = Vec2(camPos[0],camPos[2]) - layers[0].pos;
  bool changeX = false;
  bool changeZ = false;
  
  float thresh = 2 * layers[0].scale * spacing;
  if(delta[0] > thresh)
  {
    layers[0].step[0]++;
    changeX = true;
  }
  else if(delta[0] < -thresh)
  {
    layers[0].step[0]--;
    changeX = true;
  }

  if(delta[1] > thresh)
  {
    layers[0].step[1]++;
    changeZ = true;
  }
  else if(delta[1] < -thresh)
  {
    layers[0].step[1]--;
    changeZ = true;
  }

  if(changeZ || changeX)
  {
    updateLayer(0, changeX, changeZ);
  }
}
