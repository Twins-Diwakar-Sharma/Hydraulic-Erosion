#ifndef H_Gui_M3SH_H
#define H_Gui_M3SH_H

#include "glad/glad.h"
#include <vector>

class GuiMesh
{
  private:
    unsigned int vao, vbo, ebo;
    int sizeOfIndices = 0;
    void generate(std::vector<float>&, std::vector<unsigned int>&);
  public:
    GuiMesh();
    ~GuiMesh();
    void bind();
    void unbind();
    int size();
};

#endif
