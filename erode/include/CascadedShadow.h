#ifndef H_TUTI_CHHAYA_H
#define H_TUTI_CHHAYA_H

#include "Mathril.h"
#include "Projection.h"
#include "Camera.h"
#include "Light.h"
#include "ShadowFBO.h"

#define n_cascades 4

class CascadedShadow
{
private:
  bool firstUpdate=true;
  Mat4 cascOrtho[n_cascades];
  Vec3 sunPos[n_cascades];
  float partitions[n_cascades + 1];
  void cornerObjectToWorld(Vec3& corner, Camera& cam); // updates in place
  void setOrthoAndSunPos(int i, Vec3 corners[4], Vec3& origin, DirectionalLight& sun);  // updates in place
  
  void createCascades(Camera& cam, DirectionalLight& sun);

  Vec3 prevCamPos;
  Quat prevCamSpin;

  Camera sunCam;
public:
  ShadowFBO shadowFBO[n_cascades];

  CascadedShadow();
  ~CascadedShadow();
  // things to do
  // --> when cam moves, change all the orthogonal matrices
  void update(Camera& cam, DirectionalLight& sun);
  Mat4* getCascOrthos(); 
  Camera& getSunCam(int cascadeIndex);
  Mat4& getCascOrtho(int cascIndex);
  void createShadowFBOs(unsigned int width, unsigned int height);
  float getCascFarPlane(int cascIndex);
};

#endif
