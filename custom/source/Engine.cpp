#include "Engine.h"

Engine::Engine() 
{}

Engine::~Engine()
{}

void Engine::loop()
{
    clock_t prev = clock();
    double elapsed = 0, lag = 0;
    clock_t curr = clock();
    initialize();
    while(!window.close()) // ! window close
    {
        elapsed = ((double)curr - (double)prev) * 1000 / CLOCKS_PER_SEC;
        lag += elapsed;
        prev = clock();

        input();
/*      while(lag >= ms_per_update)
        {
            update();
            lag -= ms_per_update;
        }
*/
        update();
        render(lag/ms_per_update);
        curr = clock();

        framesCounter++;
        if(lag >= 1000)
        {
          lag = 0;
          currentFPS = 1000.0/(double)framesCounter;
          fpsText = "fps: " + std::to_string(framesCounter);  
          //std::cout << framesCounter << " " << std::flush;
          framesCounter = 0;
        }
  
    }
    
}    

void Engine::initialize()
{
    proj::setPerspective(60.0f,0.1f,1000.0f,window.getAspect());
    glClearColor(0.9f, 0.9f, 0.95f, 1.0f);
    //glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    //sun = DirectionalLight(Vec3(1.64,1.27,0.99));
    sun = DirectionalLight(Vec3(2.5,2.1,1.85));
    sun.rotate(-30, 90, 0);

    geoTerrain.initialize(64, 4);
    //shadowTerrain.initialize(32, 4);

    meshMap.emplace("retroCar", "retroCar");
    texMap.try_emplace("retroCar", "retroCar", TEX_DEF);
    objects.emplace_back(meshMap["retroCar"], texMap["retroCar"]);
    objects[0].setPosition(0, -2, 0);
    
    meshAnimatedMap.emplace("bunny", "BunnyWeapon");
    texMap.try_emplace("bunny", "bunny", TEX_DEF);
    objectsAnimated.emplace_back(meshAnimatedMap["bunny"], texMap["bunny"]);
    objectsAnimated[0].setRotation(0,90,0);
    objectsAnimated[0].setAnimation("angel");

    //objectsAnimated[0].animate();
    
    texMap.try_emplace("heightmap", "everest16", TEX_HDR);
    texMap.try_emplace("normalmap", "everest16Normal", TEX_HDR);
    
    cascadedShadow.createShadowFBOs(1024,1024); 
    cascadedShadow.update(cam, sun);
    
    texMap.try_emplace("handHM", "handmade_erosion", TEX_HDR); 


    auto [w, h] = window.getDimensions(); 
    hdrFBO.createHDR(w, h);
}

void Engine::input()
{
    window.handleKey(translateForward, translateSide, transVal);
    window.handleMouse(rotx, roty); 
    window.handleHold(hold);
    window.handleTerrain(updateTerrain);
    window.handleWireframe(wireframe);
    window.pollEvents(); 
}

void Engine::update()
{
  if(hold)
  {
    cam.setPosition(0,0,0);
    rotx = 0; roty = 0;
    cam.spin[0] = 1; cam.spin[1] = 0; cam.spin[2] = 0; cam.spin[3] = 0;
  }
  else
  {
    cam.rotate(rotx, roty, 0);
    rotx = 0;	roty = 0;
    cam.translate(translateForward, translateSide);
    if(updateTerrain)
    {
      geoTerrain.update(cam.position);
      //shadowTerrain.update(cam.position);
    }
    //cascadedShadow.update(cam, sun);
    objectsAnimated[0].animate();
    if(g_keyEventErode) 
    {
      g_keyEventErode = false;
      Texture* texPtr = &(texMap["handHM"]);
      erosion::startErosion(texPtr);
      //std::thread th(erosion::startErosion, texPtr);
    }
  }


  translateForward = 0; translateSide = 0;
  
}

void Engine::render(double dt)
{

  //auto [winW,winH] = window.getDimensions();
  //glViewport(0,0,winW,winH);

  glEnable(GL_DEPTH_TEST);
  hdrFBO.bind();
  auto [w,h] = hdrFBO.getDimensions();
  glViewport(0,0,w,h);


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  objectRenderer.render(objects, cam, sun);
  objectRendererAnimated.render(objectsAnimated, cam, sun);
  if(wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //geoRenderer.render(geoTerrain, cam, sun, texMap["heightmap"], texMap["normalmap"], cascadedShadow);
  geoRenderer.render(geoTerrain, cam, sun, texMap["handHM"], texMap["normalmap"], cascadedShadow);
  if(wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  hdrFBO.unbind(); //brings banding


  auto [winW,winH] = window.getDimensions();
  glViewport(0,0,winW,winH);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  postProcessRenderer.render(hdrFBO);
  window.swap();

}
