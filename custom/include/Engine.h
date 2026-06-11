#ifndef H_3N61N3_H
#define H_3N61N3_H

#include <map>
#include <iostream>
#include <time.h>
#include <vector>
#include "Win.h"
#include "Object.h"
#include "ObjectRenderer.h"
#include "Projection.h"
#include "GeoRenderer.h"
#include "GuiRenderer.h"
#include "CascadedShadow.h"
#include "ObjectRendererAnimated.h"
#include "Erosion.h"
#include <thread>
#include "PostProcessRenderer.h"


class Engine
{
private:
    float fps = 120.0f;
    float ms_per_update=1000.0f/fps;

    int framesCounter=0;
    double currentFPS = 0; 

    Win window;


    float translateForward, translateSide;
  	float transVal = 0.5f; // 0.05
  	float rotx, roty;

    void initialize();
    void update();
    void input();
    void render(double);

    std::map<std::string, Mesh> meshMap; 
    std::map<std::string, Texture> texMap;
    std::vector<Object> objects;
    ObjectRenderer objectRenderer;
    Camera cam;
    DirectionalLight sun;

    bool hold=true;
    bool wireframe=false;
    bool updateTerrain=true;

    GeoRenderer geoRenderer;
    GeoTerrain geoTerrain;
    GeoTerrain shadowTerrain;

    GuiRenderer guiRenderer;
    GuiMesh guiQuad;

    CascadedShadow cascadedShadow;
    std::string fpsText;

    std::map<std::string, MeshAnimated> meshAnimatedMap; 
    ObjectRendererAnimated objectRendererAnimated;
    std::vector<ObjectAnimated> objectsAnimated;

    PostProFBO hdrFBO;
    PostProcessRenderer postProcessRenderer;

public:
    Engine();
    ~Engine();
    void loop();
};

#endif
