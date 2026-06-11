#include "CascadedShadow.h"

CascadedShadow::CascadedShadow(): prevCamPos(0,0,0), prevCamSpin(1,0,0,0)
{
  int base = 5;
  int power = base;
  partitions[0] = 1;
  for(int i=1; i<n_cascades+1; i++)
  {
    partitions[i] = power;
    power = power * base;
  }
}

CascadedShadow::~CascadedShadow()
{}

void CascadedShadow::update(Camera& cam, DirectionalLight& sun)
{

  if(firstUpdate)
  {
    partitions[0] = proj::near;
    partitions[1] = 70;
    partitions[2] = 150;
    partitions[3] = proj::far/2.0;
    partitions[4] = proj::far;

    //partitions[n_cascades] = proj::far;
    firstUpdate = false;
  }

  if( cam.position[0] != prevCamPos[0] || cam.position[1] != prevCamPos[1] || cam.position[2] != prevCamPos[2] || 
      cam.spin[0] != prevCamSpin[0] ||  cam.spin[1] != prevCamSpin[1] ||  cam.spin[2] != prevCamSpin[2] ||  cam.spin[3] != prevCamSpin[3])
  {

    sunCam.spin = sun.getSpin();
    prevCamPos = cam.position;
    prevCamSpin = cam.spin;

    createCascades(cam, sun);
  }
}

void CascadedShadow::createCascades(Camera& cam, DirectionalLight& sun)
{
  // start with near corner, interpolate to make others
  Vec3 origin(0,0,0); 
  float oneByA = 1.0/proj::aspectRatio;
  Vec3 corners[4] = { 
        { -oneByA, 1, -proj::near},  {oneByA, 1, -proj::near},
        { -oneByA, -1, -proj::near},  {oneByA, -1, -proj::near},
  };

  cornerObjectToWorld(origin, cam);
  for(int i=0; i<4; i++)
    cornerObjectToWorld(corners[i], cam);  

  for(int i=0; i<n_cascades; i++)
  {
    setOrthoAndSunPos(i, corners, origin, sun);
  }
}


Mat4* CascadedShadow::getCascOrthos()
{
  return cascOrtho;
}

// updates in place
void CascadedShadow::cornerObjectToWorld(Vec3& corner, Camera& cam)
{
  // rotate first
  Quat posQuat(0, corner[0], corner[1], corner[2]);
  posQuat = (cam.spin) * (posQuat) * (~cam.spin);
    
  // then translate
  corner = cam.position + Vec3(posQuat[1], posQuat[2], posQuat[3]);
}

// updates in place
void CascadedShadow::setOrthoAndSunPos(int index, Vec3 corners[4], Vec3& origin, DirectionalLight& sun)
{
  float lenCorners[4];
  Vec3 unitVectors[4];
  for(int i=0; i<4; i++)
  {
    lenCorners[i] = (float)(corners[i] - origin);
    unitVectors[i] = (1.0f/lenCorners[i]) * (corners[i] - origin);
  }
  Vec3 frustomCorners[8];
  Vec3 center(0,0,0);
  for(int i=0; i<4; i++)
  {
    frustomCorners[i] = (partitions[index]/proj::near) * (corners[i] - origin)   + origin;
    frustomCorners[i+4] = (partitions[index+1]/proj::near) * (corners[i] - origin)   + origin;
    center = center + frustomCorners[i] + frustomCorners[i+4];
  }
  center = (1.0/8.0) * center;
  
  // convert corners to light space (rotate)
  for(int i=0; i<8; i++)
  {
    Quat cornerQuat(0, frustomCorners[i][0]-origin[0], frustomCorners[i][1]-origin[1], frustomCorners[i][2]-origin[2]);
    cornerQuat = (~sun.getSpin()) * cornerQuat * sun.getSpin();
    frustomCorners[i] = Vec3(cornerQuat[1], cornerQuat[2], cornerQuat[3]) + origin;
  }


  float minX = frustomCorners[0][0], maxX = minX;
  float minY = frustomCorners[0][1], maxY = minY;
  float minZ = frustomCorners[0][2], maxZ = minZ;

  for(int i=0; i<8; i++)
  {
    minX = std::min(minX, frustomCorners[i][0]);
    maxX = std::max(maxX, frustomCorners[i][0]);
    minY = std::min(minY, frustomCorners[i][1]);
    maxY = std::max(maxY, frustomCorners[i][1]);
    minZ = std::min(minZ, frustomCorners[i][2]);
    maxZ = std::max(maxZ, frustomCorners[i][2]);
  }
  float adjust = 0.0;
  minX -= adjust;
  minY -= adjust;
  minZ -= adjust;
  maxX += adjust;
  maxY += adjust;
  maxZ += adjust;
  

  cascOrtho[index][0][0] = 2.0f/(maxX - minX);
  cascOrtho[index][1][1] = 2.0f/(maxY - minY);
  cascOrtho[index][2][2] = -2.0f/(maxZ - minZ);
  cascOrtho[index][3][3] = 1; 
  cascOrtho[index][0][3] = -(maxX + minX)/(maxX - minX);
  cascOrtho[index][1][3] = -(maxY + minY)/(maxY - minY);
  cascOrtho[index][2][3] = -(maxZ + minZ)/(maxZ - minZ);

  //Vec3 center( (maxX + minX)/2.0, (maxY + minY)/2.0, (maxZ + minZ)/2.0 );
  center = Vec3( (maxX + minX)/2.0, (maxY + minY)/2.0, (maxZ + minZ)/2.0 );

  float distance = (partitions[index+1] - partitions[index]);
  //Quat centerQuat(0, (maxX + minX)/2.0, (maxY + minY)/2.0, (maxZ + minZ)/2.0);
  //centerQuat = sun.getSpin() * centerQuat * (~sun.getSpin());
  //Vec3 center(centerQuat[1], centerQuat[2], centerQuat[3]);
  distance = distance/2;
  sunPos[index] = center + distance*(-1 * sun.getDirection());
}

Camera& CascadedShadow::getSunCam(int cascadeIndex)
{
  sunCam.position = sunPos[cascadeIndex];
  return sunCam;
}


Mat4& CascadedShadow::getCascOrtho(int cascIndex)
{
  return cascOrtho[cascIndex];
}

void CascadedShadow::createShadowFBOs(unsigned int width, unsigned int height)
{
  for(int i=0; i<n_cascades; i++)
  {
    shadowFBO[i].create(width, height);
  }
}

float CascadedShadow::getCascFarPlane(int cascIndex)
{
  return partitions[cascIndex+1];
}
