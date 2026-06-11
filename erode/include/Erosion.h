#ifndef H_ydraulic_erosio_N
#define H_ydraulic_erosio_N

#include "stb_image_write.h"
#include "Texture.h"
#include "Mathril.h"
#include <random>
#include <mutex>


namespace erosion 
{

  extern std::mutex mut;

  struct Heightmap
  {
    float* data;
    int height, width, channels;

    Heightmap(float* data, int width, int height, int channels);
    float& operator[](int x, int y);
    float& operator[](Vec2&& pos);
    float& operator[](Vec2& pos);
    bool inBounds(Vec2& pos);
  };

  struct Drop
  {
    Vec2 pos;
    float speed=100;
    float volume=1.0;
    float sediment=0.0;
  };

  // world constants
  extern float depositionRate;
  extern float erosionRate;
  extern float capacityConstant;
  extern float erosionRadius;
  extern float gravity;
  extern float evapRate;
  extern float evapThresh;
  extern float inertia;
  
  float getInterpolatedHeight(Vec2& pos, Heightmap& heightmap);
  void getInterpolatedHeightAndGradient(Vec2& pos, Heightmap& heightmap, float& outH, Vec2& outGrad);
  void distributeToCorners(float weight, Vec2& pos, Heightmap& heightmap);
  void startErosion(Texture* tex);
}

#endif
