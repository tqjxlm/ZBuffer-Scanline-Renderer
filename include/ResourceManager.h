#pragma once

#include "Geometry.h"

#include <string>
#include <map>
#include <set>
#include <vector>
using namespace std;

class TextureResource
{
public:
    TextureResource()
    {
    }

    ~TextureResource()
    {
        delete texture;
    }

    string             type;
    string             path;
    Geometry::Texture *texture;
};

class GeometryResource
{
public:
    GeometryResource()
    {
    }

    ~GeometryResource()
    {
        for each (auto vertice in vertices)
        {
            delete vertice;
        }

        for each (auto face in faces)
        {
            delete face;
        }
    }

    vector<Geometry::Vertice *>  vertices;
    vector<Geometry::Face *>     faces;
    vector<TextureResource *>    textures;
};

class DrawableObject
{
public:
    DrawableObject(GeometryResource *geometry, glm::mat4 modelMatrix = glm::mat4()):
        modelMatrix(modelMatrix), useTexture(false)
    {
        this->geometries.push_back(geometry);
    }

    DrawableObject(vector<GeometryResource *> geometry, glm::mat4 modelMatrix = glm::mat4()):
        geometries(geometry), modelMatrix(modelMatrix), useTexture(false)
    {
    }

    glm::mat4                   modelMatrix;
    bool                        useTexture;
    vector<GeometryResource *>  geometries;
};

class ResourceManager
{
public:
    ResourceManager();

    ~ResourceManager();

    DrawableObject* loadCube(const glm::mat4 &modelMatrix = glm::mat4(), string id = string());

    DrawableObject* loadCube(const glm::vec4 &color, const glm::mat4 &modelMatrix = glm::mat4(), string id = string());

    DrawableObject* loadSphere(const glm::mat4 &modelMatrix = glm::mat4(), string id = string());

    DrawableObject* loadTriangle(const glm::vec4 &color = glm::vec4(255, 255, 255, 255), const glm::mat4 &modelMatrix = glm::mat4(), string id = string());

    DrawableObject* loadQuad(const glm::vec4 &color = glm::vec4(255, 255, 255, 255), const glm::mat4 &modelMatrix = glm::mat4(), string id = string());

    DrawableObject* loadModel(const string &path, const glm::mat4 &modelMatrix = glm::mat4(), string id = string());

    DrawableObject* loadTexturedQuad(const string &texturePath, const glm::mat4 &modelMatrix = glm::mat4(), string id = string());

    DrawableObject* getDrawableObject(string key)
    {
        auto  loaded = _loadedObjects.find(key);

        if (loaded == _loadedObjects.end())
        {
            return NULL;
        }
        else
        {
            return loaded->second;
        }
    }

    GeometryResource* getGeometryResource(string key)
    {
        auto  loaded = _loadedGeometries.find(key);

        if (loaded == _loadedGeometries.end())
        {
            return NULL;
        }
        else
        {
            return loaded->second;
        }
    }

    TextureResource* getTextureResource(string key)
    {
        auto  loaded = _loadedTextures.find(key);

        if (loaded == _loadedTextures.end())
        {
            return NULL;
        }
        else
        {
            return loaded->second;
        }
    }

    TextureResource* loadTextureResource(const string &path, const string &typeName, string id)
    {
        TextureResource   *resource = new TextureResource;
        Geometry::Texture *texture  = TextureFromFile(path);

        if (texture == NULL)
        {
            return NULL;
        }

        resource->texture = texture;
        resource->type    = typeName;
        resource->path    = id;

        _loadedTextures.insert_or_assign(id, resource);

        return resource;
    }

private:
    Geometry::Texture* TextureFromFile(const string &path);

    map<string, DrawableObject *>    _loadedObjects;
    map<string, GeometryResource *>  _loadedGeometries;
    map<string, TextureResource *>   _loadedTextures;
};
