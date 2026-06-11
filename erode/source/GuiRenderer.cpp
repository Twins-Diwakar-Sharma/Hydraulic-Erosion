#include "GuiRenderer.h"

GuiRenderer::GuiRenderer() : shaderProgram("gui")
{
  shaderProgram.mapUniform("scalePos");
  shaderProgram.mapUniform("albedo");
}

GuiRenderer::~GuiRenderer()
{}

void GuiRenderer::render(GuiMesh& guiMesh, unsigned int texId, Vec2 vec)
{
  shaderProgram.use();
  shaderProgram.setUniform("albedo", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texId);
  float scale = 0.1f;
  shaderProgram.setUniform("scalePos", Vec3(scale, vec[0], vec[1]));

  guiMesh.bind();
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glDrawElements(GL_TRIANGLES, guiMesh.size(), GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
  guiMesh.unbind();
}
