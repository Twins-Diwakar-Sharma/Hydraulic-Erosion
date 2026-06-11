#include "Erosion.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

// mutex
std::mutex erosion::mut;

// world constants
float erosion::depositionRate = 0.002f; // 0.0005
float erosion::erosionRate = 0.04f; // 0.04
float erosion::capacityConstant = 1.0f; // 1.0
float erosion::erosionRadius = 8; // 5
float erosion::gravity = 0.00881; // 0.00881
float erosion::evapRate = 0.05f; // 0.05
float erosion::evapThresh = 0.05; // 0.05
float erosion::inertia = 0.3f; // 0.3


void erosion::startErosion(Texture* tex)
{
  //settings
  bool exportMap = true;

  int updateAfterTicks = 500;
  int waterDrops = 10000;

  auto [width, height, channels] = tex->getDimensions();
  int bufSize = width*height*channels;
  float* data = tex->getData();
  float* bufData = tex->getData();
  Heightmap heightmapBuff[2] = {
    Heightmap(data, width, height, channels),
    Heightmap(bufData, width, height, channels)
  };


  int hm_index = 0; 
  int updateTicks = updateAfterTicks;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> w_uniDist(0, width-1);
  std::uniform_real_distribution<double> h_uniDist(0, height-1);
  
  
  std::cout << " starting erosion with " << waterDrops << std::endl;
  while(waterDrops--)
  {
    if(!updateTicks--)
    {
      tex->setData(data);
      // expect parallel thread to render
      updateTicks = updateAfterTicks;
    }

    double randX = w_uniDist(gen);
    double randY = h_uniDist(gen);
    Drop drop;
    drop.pos = Vec2(randX, randY); // check at 0,0
    Vec2 dir(0,0);
    drop.volume = 20;
    while(drop.volume > 0)
    {
      if(!heightmapBuff[hm_index].inBounds(drop.pos))
        break;

      Vec2 grad(0,0);
      float h_pos = 0;
      getInterpolatedHeightAndGradient(drop.pos, heightmapBuff[hm_index], h_pos, grad);
      dir = inertia * dir + (1.0 - inertia) * (-1 * grad); 
      float lenDir = (float)(dir);
      if(lenDir < 0.0005)
      {
          //random dir
        dir[0] =  w_uniDist(gen);
        dir[0] = 2*dir[0] - 1;
        dir[1] = h_uniDist(gen);
        dir[1] = 2*dir[1] - 1;
        lenDir = (float)(dir);
      }
      dir = (1.0/lenDir) * dir;
      Vec2 prevPos = drop.pos;
      drop.pos = drop.pos + dir;
      
      if(!heightmapBuff[hm_index].inBounds(drop.pos))
        break;
      // new H

      float new_h_pos = getInterpolatedHeight(drop.pos, heightmapBuff[hm_index]);
      float h_diff = fabs(new_h_pos - h_pos);

      float capacity = capacityConstant * h_diff * drop.speed;
      float cap_diff = capacity - drop.sediment;
      float heightFactor = 0;
      if(cap_diff > 0)
      {
        // erode
        heightFactor = -erosionRate * cap_diff;
        drop.sediment += erosionRate * cap_diff;
      }
      else 
      {
        heightFactor = + (depositionRate * (-cap_diff));
        drop.sediment -= depositionRate * (-cap_diff);
      }
      
      //distributeToCorners(heightFactor, drop.pos, heightmapBuff[1-hm_index]);
      float totalWeights = 0.0;
      int wLen = 2*erosionRadius + 1;
      std::vector<std::vector<float>> weights(wLen, std::vector<float>(wLen));
      for(float i=-erosionRadius; i<=erosionRadius; i++)
      {
        for(float j=-erosionRadius; j<=erosionRadius; j++)
        {
          Vec2 pointPos = drop.pos + Vec2(i,j);
          float len = (float)(pointPos - drop.pos);
          float w = erosionRadius - len;
          totalWeights += w;
          weights[(int)(i+erosionRadius)][(int)(j+erosionRadius)] = w;
        }
      }

      for(float i=-erosionRadius; i<=erosionRadius; i++)
      {
        for(float j=-erosionRadius; j<=erosionRadius; j++)
        {
          Vec2 pointPos = drop.pos + Vec2(i,j);
          if(!heightmapBuff[1-hm_index].inBounds(pointPos))
            continue;
          float w = weights[(int)(i+erosionRadius)][(int)(j+erosionRadius)];
          w = w/totalWeights;
          heightmapBuff[1-hm_index][pointPos] += w * heightFactor;
        }
      }
      
      // update speed
      float sqSp = drop.speed * drop.speed - gravity * abs(h_diff);
      sqSp = sqSp <= 0.0 ? 0.0 : sqSp;
      drop.speed = sqrt(sqSp);

        

      // evaporate
      drop.volume = drop.volume * ( 1.0 - evapRate);
      float evaporated = drop.volume * evapRate;
      evaporated = evaporated > evapThresh ? evaporated : evapThresh;
      //drop.volume -= evaporated;
      drop.volume -= 0.05;

    }
    // copy new buff into old one
    memcpy(heightmapBuff[hm_index].data, heightmapBuff[1-hm_index].data, sizeof(float) * width * height * channels);
    std::cout << " ." << std::flush;
  }
  std::cout << std::endl; 
  tex->setData(data);

  
  std::cout << "successfully ended erosion" << std::endl;
  if(exportMap)
  {
    std::cout << "exporting new heightmap " << std::endl;
    std::string exportTexName = tex->getName() + "_erosion.png";
    stbi_write_hdr(exportTexName.c_str(), width, height, channels, data);
  }

  delete [] data;
  delete [] bufData;
}

erosion::Heightmap::Heightmap(float* data, int width, int height, int channels):
  data(data), width(width), height(height), channels(channels)
{ 
}

float erosion::getInterpolatedHeight(Vec2& pos, Heightmap& heightmap)
{
  Vec2 cell00 = Vec2((int)pos[0], (int)pos[1]); 
  Vec2 cell01 = Vec2((int)pos[0], (int)pos[1]) + Vec2(0,1);
  Vec2 cell10 = Vec2((int)pos[0], (int)pos[1]) + Vec2(1,0); 
  Vec2 cell11 = Vec2((int)pos[0], (int)pos[1]) + Vec2(1,1); 

  if(! (heightmap.inBounds(cell00) && heightmap.inBounds(cell01) && heightmap.inBounds(cell10) && heightmap.inBounds(cell11)) )
    return 0;

  Vec2 fractPos = pos - cell00; // between 0 and 1 hopefully
  float h_pos = heightmap[cell11]*fractPos[0]*fractPos[1] + heightmap[cell01]*(1.0-fractPos[0])*fractPos[1]
    + heightmap[cell10]*fractPos[0]*(1.0-fractPos[1]) + heightmap[cell00]*(1.0-fractPos[0])*(1.0-fractPos[1]);
  return h_pos;
}

void erosion::getInterpolatedHeightAndGradient(Vec2& pos, Heightmap& heightmap, float& outH, Vec2& outGrad)
{

  Vec2 cell00 = Vec2((int)pos[0], (int)pos[1]); 
  Vec2 cell01 = Vec2((int)pos[0], (int)pos[1]) + Vec2(0,1);
  Vec2 cell10 = Vec2((int)pos[0], (int)pos[1]) + Vec2(1,0); 
  Vec2 cell11 = Vec2((int)pos[0], (int)pos[1]) + Vec2(1,1); 

  if(! (heightmap.inBounds(cell00) && heightmap.inBounds(cell01) && heightmap.inBounds(cell10) && heightmap.inBounds(cell11)) )
    return;

  Vec2 fractPos = pos - cell00; // between 0 and 1 hopefully
  outH = heightmap[cell11]*fractPos[0]*fractPos[1] + heightmap[cell01]*(1.0-fractPos[0])*fractPos[1]
    + heightmap[cell10]*fractPos[0]*(1.0-fractPos[1]) + heightmap[cell00]*(1.0-fractPos[0])*(1.0-fractPos[1]);
  float gradX = (heightmap[cell11] - heightmap[cell01])*fractPos[1] + (heightmap[cell10] - heightmap[cell00])*(1.0-fractPos[1]);
  float gradY = (heightmap[cell11] - heightmap[cell10])*fractPos[0] + (heightmap[cell01] - heightmap[cell00])*(1.0-fractPos[0]);

  outGrad[0] = gradX;
  outGrad[1] = gradY;
}

void erosion::distributeToCorners(float weight, Vec2& pos,  Heightmap& heightmap)
{
  Vec2 cell00 = Vec2((int)pos[0], (int)pos[1]); 
  Vec2 cell01 = Vec2((int)pos[0], (int)pos[1]) + Vec2(0,1);
  Vec2 cell10 = Vec2((int)pos[0], (int)pos[1]) + Vec2(1,0); 
  Vec2 cell11 = Vec2((int)pos[0], (int)pos[1]) + Vec2(1,1); 

  if(! (heightmap.inBounds(cell00) && heightmap.inBounds(cell01) && heightmap.inBounds(cell10) && heightmap.inBounds(cell11)) )
    return;

  Vec2 fractPos = pos - cell00; // between 0 and 1 hopefully
  
  heightmap[cell00] += weight * (1-fractPos[0]) * (1-fractPos[1]);
  heightmap[cell10] += weight * (fractPos[0]) * (1-fractPos[1]);
  heightmap[cell01] += weight * (1-fractPos[0]) * (fractPos[1]);
  heightmap[cell11] += weight * (fractPos[0]) * (fractPos[1]);
}

float& erosion::Heightmap::operator[](int x, int y)
{
  return data[ (y * width + x) * channels]; // only sending red
}

float& erosion::Heightmap::operator[](Vec2& pos)
{
  return data[ ((int)(pos[1]) * width + (int)(pos[0])) * channels ];
}

float& erosion::Heightmap::operator[](Vec2&& pos)
{
  return data[ ((int)(pos[1]) * width + (int)(pos[0])) * channels ];
}

bool erosion::Heightmap::inBounds(Vec2& pos)
{
  return pos[0] >= 0 && pos[0] < width 
     && pos[1] >= 0 && pos[1] < height;
}
