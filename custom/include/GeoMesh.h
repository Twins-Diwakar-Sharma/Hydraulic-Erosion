#ifndef H_G30_M3SH_H
#define H_G30_M3SH_H

#include <vector>
#include "glad/glad.h"
#include <iostream>

#define GRID_FULL 0
#define GRID_HOLLOW 1  
#define STRIP 2
#define STRIP_ALT 3

class GeoMesh
{
  private:
    unsigned int vao, vbo, ebo; 
    int sizeOfIndices=0;
    void generate(std::vector<float>&, std::vector<unsigned int>&);

  public:
    GeoMesh();
    GeoMesh(int type, int blocksWidth, float spacing);
    ~GeoMesh();
    void bind();
    void unbind();
    int size();
    void create(int type, int blocksWidth, float spacing);
};

#endif
