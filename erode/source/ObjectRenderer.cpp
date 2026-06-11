#include "ObjectRenderer.h"

ObjectRenderer::ObjectRenderer() : shaderProgram("objects/objects"), shadowProgram("objects/shadowObjects")
{
    shaderProgram.mapUniform("projection");
    shaderProgram.mapUniform("transform");
    shaderProgram.mapUniform("albedo");
    shaderProgram.mapCameraUniform("cam");
    shaderProgram.mapDirectionalLightUniform("sun");

    shadowProgram.mapUniform("projection");
    shadowProgram.mapUniform("transform");
    shadowProgram.mapUniform("albedo");
    shadowProgram.mapCameraUniform("cam");
}

ObjectRenderer::~ObjectRenderer()
{
}

void ObjectRenderer::render(std::vector<Object>& objects, Camera& cam, DirectionalLight& sun)
{
    shaderProgram.use();
    shaderProgram.setUniform("projection",proj::perspective);
    shaderProgram.setUniform("albedo",0);
    shaderProgram.setUniform("cam", cam);
    shaderProgram.setUniform("sun", sun);

    for(int i=0; i<objects.size(); i++)
    {
            shaderProgram.setUniform("transform",objects[i].getTransform());
            objects[i].bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, objects[i].getTextureId());
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glDrawElements(GL_TRIANGLES, objects[i].size(), GL_UNSIGNED_INT, 0);
            glDisableVertexAttribArray(2);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(0);
            objects[i].unbind();
    }
    
    shaderProgram.unuse();
}


void ObjectRenderer::renderShadows(std::vector<Object>& objects, CascadedShadow& cascShadow, int cascIndex, Camera& cam)
{
    shadowProgram.use();
    shadowProgram.setUniform("projection", cascShadow.getCascOrtho(cascIndex));
    shadowProgram.setUniform("albedo",0);
    shadowProgram.setUniform("cam", cascShadow.getSunCam(cascIndex));

    for(int i=0; i<objects.size(); i++)
    {
        shadowProgram.setUniform("transform",objects[i].getTransform());
        objects[i].bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, objects[i].getTextureId());
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawElements(GL_TRIANGLES, objects[i].size(), GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        objects[i].unbind();
    }
    
    shadowProgram.unuse();
}
