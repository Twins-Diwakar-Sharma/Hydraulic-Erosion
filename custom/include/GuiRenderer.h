#ifndef H_GU1_Renderer_H
#define H_GU1_Renderer_H

#include "GuiMesh.h"
#include "ShaderProgram.h"

class GuiRenderer
{
private:
  ShaderProgram shaderProgram;
public:
  GuiRenderer();
  ~GuiRenderer();
  void render(GuiMesh& guiMesh, unsigned int texId, Vec2 vec);
};

#endif
