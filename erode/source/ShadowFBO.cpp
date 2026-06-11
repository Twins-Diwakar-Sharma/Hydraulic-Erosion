#include "ShadowFBO.h"

ShadowFBO::ShadowFBO()
{
  glGenFramebuffers(1, &fbo);
}

ShadowFBO::~ShadowFBO()
{
  glDeleteFramebuffers(1, &fbo); 
}

void ShadowFBO::create(unsigned int width, unsigned int height)
{
  this->width = width;
  this->height = height;

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  
  float white[] = {1,1,1,1};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void ShadowFBO::bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void ShadowFBO::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int ShadowFBO::getTextureId()
{
  return texId;
}

std::tuple<unsigned int, unsigned int> ShadowFBO::getDimensions()
{
  return {width, height};
}

