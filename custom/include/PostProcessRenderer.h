#ifndef H_POST_PROCESSOR_H
#define H_POST_PROCESSOR_H

#include "GuiMesh.h"
#include "ShaderProgram.h"
#include "PostProFBO.h"

class PostProcessRenderer
{
private:
  ShaderProgram shaderProgram;
  GuiMesh guiMesh;
public:
  PostProcessRenderer();
  ~PostProcessRenderer();
  void render(PostProFBO& fbo);
};

#endif
