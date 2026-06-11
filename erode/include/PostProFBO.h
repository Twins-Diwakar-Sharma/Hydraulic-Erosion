#ifndef H_POST_PROCESSING_FBO_H
#define H_POST_PROCESSING_FBO_H

#include "glad/glad.h"
#include <tuple>

class PostProFBO
{
private:
  unsigned int fbo;
  unsigned int rbo;
  unsigned int texId, depthId;
  unsigned int width, height;
public:
  PostProFBO();
  ~PostProFBO();

  void create(unsigned int width, unsigned int height);
  void createDepth(unsigned int width, unsigned int height);
  void createHDR(unsigned int width, unsigned int height);

  void bind();
  void unbind();

  unsigned int getTextureId();
  unsigned int getDepthId();
  std::tuple<unsigned int, unsigned int> getDimensions();

};

#endif
