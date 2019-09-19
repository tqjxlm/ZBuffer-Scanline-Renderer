#include "ResourceManager.h"

#include <SOIL\SOIL.h>

#include <iostream>
#include <vector>
using namespace std;

#include "Model.h"
#include "SimpleResources.h"

ResourceManager::ResourceManager()
{}

ResourceManager::~ResourceManager()
{
    for (auto& resource: loadedGeometries_)
    {
        delete resource.second;
    }

    for (auto& resource: loadedTextures_)
    {
        delete resource.second;
    }

    for (auto& object: loadedObjects_)
    {
        delete object.second;
    }
}

DrawableObject * ResourceManager::loadCube(const glm::mat4& modelMatrix, string id)
{
    auto loadedResource = loadedGeometries_.find("Cube");

    // If resource does not exists
    if (loadedResource == loadedGeometries_.end())
    {
        GeometryResource* resource = new GeometryResource;

        // Process: Quad -> Point
        for (int quad = 0; quad < 6; quad++)
        {
            for (int point = 0; point < 4; point++)
            {
                Geometry::Vertice* vertice = new Geometry::Vertice;
                vertice->position =
                    glm::vec3(cubeVertices[quad][point][0], cubeVertices[quad][point][1], cubeVertices[quad][point][2]);
                colorCpy(vertice->color, cubeColors[quad]);
                resource->vertices.push_back(vertice);
            }

            vector<int> indices  = { quad * 4 + 0, quad * 4 + 1, quad * 4 + 2, quad * 4 + 3 };
            Geometry::Face* face = new Geometry::Face(resource->vertices);
            face->indices = indices;
            resource->faces.push_back(face);
        }

        // Record object
        loadedResource = loadedGeometries_.insert_or_assign(string("Cube"), resource).first;
    }

    // Generate drawable object with the loaded resource
    if (id.empty())
    {
        static int count = 0;
        id = "Cube" + (count++);
    }

    auto inserted = loadedObjects_.insert_or_assign(id, new DrawableObject(loadedResource->second, modelMatrix));

    return inserted.first->second;
}

DrawableObject * ResourceManager::loadCube(const glm::vec4& color, const glm::mat4& modelMatrix, string id)
{
    unsigned char intColor[4] = {
        static_cast<unsigned char>(color.r),
        static_cast<unsigned char>(color.g),
        static_cast<unsigned char>(color.b),
        static_cast<unsigned char>(color.a) };
    auto object = loadCube(modelMatrix, id);

    for (auto vertice: object->geometries[0]->vertices)
    {
        colorCpy(vertice->color, intColor);
    }

    return object;
}

DrawableObject * ResourceManager::loadSphere(const glm::mat4& modelMatrix, string id)
{
    return nullptr;
}

DrawableObject * ResourceManager::loadTriangle(const glm::vec4& color, const glm::mat4& modelMatrix, string id)
{
    // A unique id
    if (id.empty())
    {
        static int count = 0;
        id = "Triangle" + (count++);
    }

    unsigned char intColor[4] = {
        static_cast<unsigned char>(color.r),
        static_cast<unsigned char>(color.g),
        static_cast<unsigned char>(color.b),
        static_cast<unsigned char>(color.a) };

    // Generate a new polygon resource
    GeometryResource* resource = new GeometryResource;

    for (int i = 0; i < sizeof(triangleVertices) / sizeof(triangleVertices[0]); i++)
    {
        Geometry::Vertice* vertice = new Geometry::Vertice;
        vertice->position = glm::vec3(triangleVertices[i][0], triangleVertices[i][1], triangleVertices[i][2]);
        colorCpy(vertice->color, intColor);
        resource->vertices.push_back(vertice);
    }

    Geometry::Face* face = new Geometry::Face(resource->vertices);
    face->indices = triIndice;
    resource->faces.push_back(face);

    auto loadedResource = loadedGeometries_.insert_or_assign(string("Triangle"), resource).first;

    // Generate drawable object with the loaded resource
    auto inserted = loadedObjects_.insert_or_assign(id, new DrawableObject(loadedResource->second, modelMatrix));

    return inserted.first->second;
}

DrawableObject * ResourceManager::loadQuad(const glm::vec4& color, const glm::mat4& modelMatrix, string id)
{
    // A unique id
    if (id.empty())
    {
        static int count = 0;
        id = "Quad" + (count++);
    }

    unsigned char triColor[4] = {
        static_cast<unsigned char>(color.r),
        static_cast<unsigned char>(color.g),
        static_cast<unsigned char>(color.b),
        static_cast<unsigned char>(color.a) };

    // Generate a new polygon resource
    GeometryResource* resource = new GeometryResource;

    for (int i = 0; i < sizeof(quadVertices) / sizeof(quadVertices[0]); i++)
    {
        Geometry::Vertice* vertice = new Geometry::Vertice;
        vertice->position = glm::vec3(quadVertices[i][0], quadVertices[i][1], quadVertices[i][2]);
        colorCpy(vertice->color, triColor);
        resource->vertices.push_back(vertice);
    }

    Geometry::Face* face = new Geometry::Face(resource->vertices);
    face->indices = quadIndice;
    resource->faces.push_back(face);

    auto loadedResource = loadedGeometries_.insert_or_assign(string("Quad"), resource).first;

    // Generate drawable object with the loaded resource
    auto inserted = loadedObjects_.insert_or_assign(id, new DrawableObject(loadedResource->second, modelMatrix));

    return inserted.first->second;
}

DrawableObject * ResourceManager::loadModel(const string& path, const glm::mat4& modelMatrix, string id)
{
    if (id.empty())
    {
        id = path;
    }

    auto loadedObject = loadedObjects_.find(id);
    DrawableObject* loadedModel;

    // If resource does not exists
    if (loadedObject == loadedObjects_.end())
    {
        Model model(this);
        model.loadModel(path);

        loadedModel = model.getDrawableObject();
    }
    else
    {
        // Just make a copy of it
        loadedModel = new DrawableObject(*loadedObject->second);
    }

    if (loadedModel == nullptr)
    {
        cout << "Unable to load resource: " << path << endl;

        return nullptr;
    }

    // Generate drawable object with the loaded resource
    loadedModel->modelMatrix = modelMatrix;
    auto inserted = loadedObjects_.insert_or_assign(id, loadedModel);

    return inserted.first->second;
}

DrawableObject * ResourceManager::loadTexturedQuad(const string& texturePath, const glm::mat4& modelMatrix, string id)
{
    DrawableObject* loadedQuad = loadQuad(glm::vec4(255, 255, 255, 255), modelMatrix, id);

    for (int i = 0; i < sizeof(texCoords) / sizeof(texCoords[0]); i++)
    {
        loadedQuad->geometries[0]->vertices[i]->texCoord = glm::vec2(texCoords[i][0], texCoords[i][1]);
    }

    loadedQuad->geometries[0]->textures.push_back(loadTextureResource(texturePath, "texture_diffuse", texturePath));
    loadedQuad->useTexture = true;

    return loadedQuad;
}

Geometry::Texture * ResourceManager::TextureFromFile(const string& path)
{
    cout << "loading texture: " << path << endl;

    int width, height, channel;
    unsigned char* image;
    try
    {
        image = SOIL_load_image(path.c_str(), &width, &height, &channel, SOIL_LOAD_RGBA);
    }
    catch (...)
    {
        image = nullptr;
    }
    if (image != nullptr)
    {
        return new Geometry::Texture(image, width, height, 4);
    }
    else
    {
        return nullptr;
    }
}
