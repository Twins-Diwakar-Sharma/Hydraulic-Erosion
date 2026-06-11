#include "GeoRenderer.h"


GeoRenderer::GeoRenderer() : shaderProgram("geo/geo"), shadowProgram("geo/shadowGeo")
{
  shaderProgram.mapUniform("projection");
  shaderProgram.mapCameraUniform("cam");
  shaderProgram.mapDirectionalLightUniform("sun");
  shaderProgram.mapUniform("worldPos");
  shaderProgram.mapUniform("center");
  shaderProgram.mapUniform("stripScale");
  shaderProgram.mapUniform("debugCol");
  shaderProgram.mapUniform("scale");
  shaderProgram.mapUniform("config");
  shaderProgram.mapUniform("heightmap");
  shaderProgram.mapUniform("normalmap");
  
  for(int cascIdx=0; cascIdx<n_cascades; cascIdx++)
  {
    shaderProgram.mapUniform("cascShadowmap[" + std::to_string(cascIdx) + "]");
    shaderProgram.mapCameraUniform("cascSunCam[" + std::to_string(cascIdx) + "]");
    shaderProgram.mapUniform("cascOrtho[" + std::to_string(cascIdx) + "]");
    shaderProgram.mapUniform("cascFarPlane[" + std::to_string(cascIdx) + "]");
  }

  shadowProgram.mapUniform("projection");
  shadowProgram.mapCameraUniform("cam");
  shadowProgram.mapDirectionalLightUniform("sun");
  shadowProgram.mapUniform("worldPos");
  shadowProgram.mapUniform("center");
  shadowProgram.mapUniform("stripScale");
  shadowProgram.mapUniform("debugCol");
  shadowProgram.mapUniform("scale");
  shadowProgram.mapUniform("config");
  shadowProgram.mapUniform("heightmap");
}

GeoRenderer::~GeoRenderer()
{
}

void GeoRenderer::render(GeoMesh& geoMesh, Camera& cam, DirectionalLight& sun)
{
}

void GeoRenderer::render(GeoTerrain& terrain, Camera& cam, DirectionalLight& sun, Texture& heightmap, Texture& normalmap, CascadedShadow& cascadedShadow)
{
  shaderProgram.use();
  shaderProgram.setUniform("projection", proj::perspective);
  shaderProgram.setUniform("cam", cam); 
  shaderProgram.setUniform("sun", sun);
  shaderProgram.setUniform("stripScale", Vec2(1,1));
  shaderProgram.setUniform("debugCol", 0);
  shaderProgram.setUniform("config", Vec2(terrain.blocksWidth, terrain.spacing));
  shaderProgram.setUniform("heightmap", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, heightmap.getTextureId());
  shaderProgram.setUniform("normalmap", 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normalmap.getTextureId()); 

  for(int cascIdx = 0; cascIdx<n_cascades; cascIdx++)
  {
    shaderProgram.setUniform("cascShadowmap[" + std::to_string(cascIdx) + "]", 2 + cascIdx);
    glActiveTexture(GL_TEXTURE2 + cascIdx);
    glBindTexture(GL_TEXTURE_2D, cascadedShadow.shadowFBO[cascIdx].getTextureId());
    shaderProgram.setUniform("cascSunCam[" + std::to_string(cascIdx) + "]", cascadedShadow.getSunCam(cascIdx));
    shaderProgram.setUniform("cascOrtho[" + std::to_string(cascIdx) + "]", cascadedShadow.getCascOrtho(cascIdx));
    shaderProgram.setUniform("cascFarPlane[" + std::to_string(cascIdx) + "]", cascadedShadow.getCascFarPlane(cascIdx));
  }

  terrain.gridFull.bind();
  shaderProgram.setUniform("worldPos", terrain.layers[0].pos);
  shaderProgram.setUniform("center", terrain.layers[0].pos);
  shaderProgram.setUniform("scale", terrain.layers[0].scale);
  glEnableVertexAttribArray(0);
  glDrawElements(GL_TRIANGLES, terrain.gridFull.size(), GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);
  terrain.gridFull.unbind();

  terrain.gridHollow.bind();
  for(int i=1; i<n_layers; i++)
  {
    shaderProgram.setUniform("worldPos", terrain.layers[i].pos);
    shaderProgram.setUniform("center", terrain.layers[i].pos);
    shaderProgram.setUniform("scale", terrain.layers[i].scale);
    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, terrain.gridHollow.size(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
  }
  terrain.gridHollow.unbind();
  
  for(int i=1; i<n_layers; i++)
  {
    shaderProgram.setUniform("debugCol", 1);
    shaderProgram.setUniform("scale", terrain.layers[i].scale);
    shaderProgram.setUniform("stripScale", 2*terrain.layers[i-1].step);
    shaderProgram.setUniform("worldPos", terrain.layers[i].stripPos);
    shaderProgram.setUniform("center", terrain.layers[i].pos);
    if(terrain.layers[i-1].step[0] * terrain.layers[i-1].step[1] > 0)
      terrain.strip.bind();
    else
      terrain.stripAlt.bind();
    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, terrain.strip.size(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    terrain.strip.unbind();
  }
  shaderProgram.unuse();
}


void GeoRenderer::renderShadows(GeoTerrain& terrain, CascadedShadow& cascShadow, int cascIndex, Camera& cam, Texture& heightmap)
{

  shadowProgram.use();

  shadowProgram.setUniform("projection", cascShadow.getCascOrtho(cascIndex));
  shadowProgram.setUniform("cam", cascShadow.getSunCam(cascIndex)); 
  shadowProgram.setUniform("stripScale", Vec2(1,1));
  shadowProgram.setUniform("debugCol", 0);
  shadowProgram.setUniform("config", Vec2(terrain.blocksWidth, terrain.spacing));
  shadowProgram.setUniform("heightmap", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, heightmap.getTextureId());
  
  
  terrain.gridFull.bind();
  shadowProgram.setUniform("worldPos", terrain.layers[0].pos);
  shadowProgram.setUniform("center", terrain.layers[0].pos);
  shadowProgram.setUniform("scale", terrain.layers[0].scale);
  glEnableVertexAttribArray(0);
  glDrawElements(GL_TRIANGLES, terrain.gridFull.size(), GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);
  terrain.gridFull.unbind();

  terrain.gridHollow.bind();
  for(int i=1; i<n_layers; i++)
  {
    shadowProgram.setUniform("worldPos", terrain.layers[i].pos);
    shadowProgram.setUniform("center", terrain.layers[i].pos);
    shadowProgram.setUniform("scale", terrain.layers[i].scale);
    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, terrain.gridHollow.size(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
  }
  terrain.gridHollow.unbind();
  
  for(int i=1; i<n_layers; i++)
  {
    shadowProgram.setUniform("debugCol", 1);
    shadowProgram.setUniform("scale", terrain.layers[i].scale);
    shadowProgram.setUniform("stripScale", 2*terrain.layers[i-1].step);
    shadowProgram.setUniform("worldPos", terrain.layers[i].stripPos);
    shadowProgram.setUniform("center", terrain.layers[i].pos);
    if(terrain.layers[i-1].step[0] * terrain.layers[i-1].step[1] > 0)
      terrain.strip.bind();
    else
      terrain.stripAlt.bind();
    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, terrain.strip.size(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    terrain.strip.unbind();
  }
  shadowProgram.unuse();
}
