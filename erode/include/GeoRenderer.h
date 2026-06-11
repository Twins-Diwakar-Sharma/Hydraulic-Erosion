#ifndef H_GEO_R3ND3R3R_H
#define H_GEO_R3ND3R3R_H

#include "Mathril.h"
#include "ShaderProgram.h"
#include "GeoTerrain.h"
#include "Projection.h"
#include <vector>
#include "Texture.h"
#include "CascadedShadow.h"


class GeoRenderer
{
private:
    ShaderProgram shaderProgram, shadowProgram;
public:
    GeoRenderer();
    ~GeoRenderer();  
    void render(GeoMesh&, Camera&, DirectionalLight&);
    void render(GeoTerrain&, Camera&, DirectionalLight&, Texture& heightmap, Texture& normalmap, CascadedShadow&);
    void renderShadows(GeoTerrain&, CascadedShadow& cascShadow, int index, Camera& cam, Texture& heightmap);
};


#endif
