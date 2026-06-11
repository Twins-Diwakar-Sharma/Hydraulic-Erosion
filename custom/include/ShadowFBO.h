#ifndef H_Shadow_FBO_H
#define H_Shadow_FBO_H

#include "glad/glad.h"
#include <tuple>

class ShadowFBO
{
  private:
    unsigned int fbo;
    unsigned int texId;

    unsigned int width, height;
  public:
    ShadowFBO();
    ~ShadowFBO();
    
    void create(unsigned int width, unsigned int height);
    void bind();
    void unbind();

    unsigned int getTextureId();
    std::tuple<unsigned int, unsigned int> getDimensions();
};

#endif 
