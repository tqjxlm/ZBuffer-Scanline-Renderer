#pragma once

#include "Geometry.h"

#include <string>
#include <unordered_map>
#include <vector>

class TextureResource {
public:

    TextureResource()
    {}

    ~TextureResource()
    {
        delete texture;
    }

    std::string type;
    std::string path;
    Geometry::Texture* texture;
};

class GeometryResource {
public:

    GeometryResource()
    {}

    ~GeometryResource()
    {
        for (auto vertice: vertices)
        {
            delete vertice;
        }

        for (auto face: faces)
        {
            delete face;
        }
    }

    std::vector<Geometry::Vertice *>vertices;
    std::vector<Geometry::Face *>faces;
    std::vector<TextureResource *>textures;
};

class DrawableObject {
public:

    DrawableObject(GeometryResource* geometry, glm::mat4 modelMatrix = glm::mat4()) :
        modelMatrix(modelMatrix), useTexture(false)
    {
        this->geometries.push_back(geometry);
    }

    DrawableObject(std::vector<GeometryResource *>geometry, glm::mat4 modelMatrix = glm::mat4()) :
        geometries(geometry), modelMatrix(modelMatrix), useTexture(false)
    {}

    glm::mat4 modelMatrix;
    bool useTexture;
    std::vector<GeometryResource *>geometries;
};

class ResourceManager {
public:

    ResourceManager();

    ~ResourceManager();

    DrawableObject* loadCube(const glm::mat4& modelMatrix = glm::mat4(),
                             std::string      id          = std::string());

    DrawableObject* loadCube(const glm::vec4& color,
                             const glm::mat4& modelMatrix = glm::mat4(),
                             std::string      id          = std::string());

    DrawableObject* loadSphere(const glm::mat4& modelMatrix = glm::mat4(),
                               std::string      id          = std::string());

    DrawableObject* loadTriangle(const glm::vec4& color       = glm::vec4(255, 255, 255, 255),
                                 const glm::mat4& modelMatrix = glm::mat4(),
                                 std::string id               = std::string());

    DrawableObject* loadQuad(const glm::vec4& color       = glm::vec4(255, 255, 255, 255),
                             const glm::mat4& modelMatrix = glm::mat4(),
                             std::string id               = std::string());

    DrawableObject* loadModel(const std::string& path,
                              const glm::mat4  & modelMatrix = glm::mat4(),
                              std::string        id          = std::string());

    DrawableObject* loadTexturedQuad(const std::string& texturePath,
                                     const glm::mat4  & modelMatrix = glm::mat4(),
                                     std::string        id          = std::string());

    DrawableObject* getDrawableObject(std::string key)
    {
        auto loaded = loadedObjects_.find(key);

        if (loaded == loadedObjects_.end())
        {
            return nullptr;
        }
        else
        {
            return loaded->second;
        }
    }

    GeometryResource* getGeometryResource(std::string key)
    {
        auto loaded = loadedGeometries_.find(key);

        if (loaded == loadedGeometries_.end())
        {
            return nullptr;
        }
        else
        {
            return loaded->second;
        }
    }

    TextureResource* getTextureResource(std::string key)
    {
        auto loaded = loadedTextures_.find(key);

        if (loaded == loadedTextures_.end())
        {
            return nullptr;
        }
        else
        {
            return loaded->second;
        }
    }

    TextureResource* loadTextureResource(const std::string& path, const std::string& typeName, std::string id)
    {
        TextureResource  * resource = new TextureResource;
        Geometry::Texture* texture  = TextureFromFile(path);

        if (texture == nullptr)
        {
            return nullptr;
        }

        resource->texture = texture;
        resource->type    = typeName;
        resource->path    = id;

        loadedTextures_.insert_or_assign(id, resource);

        return resource;
    }

private:

    Geometry::Texture* TextureFromFile(const std::string& path);

private:

    std::unordered_map<std::string, DrawableObject *>loadedObjects_;
    std::unordered_map<std::string, GeometryResource *>loadedGeometries_;
    std::unordered_map<std::string, TextureResource *>loadedTextures_;
};
