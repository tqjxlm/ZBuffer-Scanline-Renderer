#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

class GeometryResource;
class TextureResource;
class DrawableObject;
class ResourceManager;

class Model {
public:

    Model()
    {}

    Model(ResourceManager* manager) :
        parentManager(manager)
    {}

    Model(char* path)
    {
        this->loadModel(path);
    }

    void            loadModel(std::string path);

    DrawableObject* getDrawableObject()
    {
        return object;
    }

private:

    void processNode(aiNode       * node,
                     const aiScene* scene);

    void processMesh(aiMesh       * mesh,
                     const aiScene* scene);

    void loadMaterialTextures(aiMaterial      * mat,
                              aiTextureType     type,
                              std::string       typeName,
                              GeometryResource* geometryRc);

private:

    ResourceManager* parentManager;
    DrawableObject* object;
    std::vector<GeometryResource *>geometryRcs;
    std::string directory;
    int totalTextureLoaded = 0;
};
