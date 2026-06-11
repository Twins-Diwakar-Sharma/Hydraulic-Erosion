#ifndef H_GEO_CLIP_MAP_TERRAIN_H
#define H_GEO_CLIP_MAP_TERRAIN_H

#include "Mathril.h"
#include "GeoMesh.h"

struct Layer
{
  Vec2 step;
  Vec2 pos; // add with steps in renderer
  int scale = 1;
  Vec2 stripPos;
};

#define n_layers 4
class GeoTerrain
{
  private:
    void updateLayer(int i, bool changeX, bool changeZ);
  public:
    float spacing = 1;
    int blocksWidth = 4;
    GeoTerrain();
    ~GeoTerrain();
    void initialize(int blocksWidth, float spacing);
    void update(Vec3& camPos);
    GeoMesh gridFull, gridHollow, strip, stripAlt;
    Layer layers[n_layers];
};

#endif
