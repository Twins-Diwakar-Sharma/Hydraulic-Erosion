#include "PostProcessRenderer.h"

PostProcessRenderer::PostProcessRenderer() : shaderProgram("post")
{
  shaderProgram.mapUniform("albedo");
  shaderProgram.mapUniform("depth");
  shaderProgram.mapUniform("screenDim");
}

PostProcessRenderer::~PostProcessRenderer()
{}

void PostProcessRenderer::render(PostProFBO& fbo)
{
  shaderProgram.use();
  shaderProgram.setUniform("albedo", 0);
  shaderProgram.setUniform("depth", 1);
  auto [scrnW, scrnH] = fbo.getDimensions();
  shaderProgram.setUniform("screenDim", Vec2(scrnW, scrnH));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fbo.getTextureId());
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, fbo.getDepthId());
  guiMesh.bind();
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glDrawElements(GL_TRIANGLES, guiMesh.size(), GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
  guiMesh.unbind();
}
