#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
using namespace std;

class GeometryResource;
class TextureResource;
class DrawableObject;
class ResourceManager;

class Model
{
public:
    Model()
    {
    }

    Model(ResourceManager *manager):
        parentManager(manager)
    {
    }

    Model(char *path)
    {
        this->loadModel(path);
    }

    void            loadModel(string path);

    DrawableObject* getDrawableObject()
    {
        return object;
    }

private:
    void  processNode(aiNode *node, const aiScene *scene);

    void  processMesh(aiMesh *mesh, const aiScene *scene);

    void  loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                               string typeName, GeometryResource *geometryRc);

private:
    ResourceManager            *parentManager;
    DrawableObject             *object;
    vector<GeometryResource *>  geometryRcs;
    string                      directory;
    int                         totalTextureLoaded = 0;
};
