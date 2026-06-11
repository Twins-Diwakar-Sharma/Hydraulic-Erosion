#ifndef H_R3nD3r3R_H
#define H_R3nD3r3R_H


#include "Mathril.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Projection.h"
#include "Object.h"
#include <vector>
#include "CascadedShadow.h"

class ObjectRenderer
{
private:
    ShaderProgram shaderProgram, shadowProgram;
public:
    ObjectRenderer();
    ~ObjectRenderer();  
    void render(std::vector<Object>& objects, Camera& cam, DirectionalLight& sun);
    void renderShadows(std::vector<Object>& objects, CascadedShadow& cascShadow, int index, Camera& cam);
};

#endif
