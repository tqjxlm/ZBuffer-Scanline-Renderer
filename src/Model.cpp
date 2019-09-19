#include "Model.h"

#include <iostream>
using namespace std;

#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include "Geometry.h"
#include "ResourceManager.h"

void Model::loadModel(string path)
{
    Assimp::Importer import;
    auto readOptions = aiProcess_OptimizeMeshes |
                       aiProcess_OptimizeGraph |
                       aiProcess_Triangulate |
                       aiProcess_SplitLargeMeshes |
                       aiProcess_ImproveCacheLocality |
                       aiProcess_RemoveRedundantMaterials |
                       aiProcess_JoinIdenticalVertices;
    const aiScene* scene = import.ReadFile(path, readOptions);

    if (!scene || (scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        drawable_ = nullptr;

        return;
    }

    this->directory_ = path.substr(0, path.find_last_of('/'));

    this->processNode(scene->mRootNode, scene);

    drawable_ = new DrawableObject(geometryRcs_);

    if (totalTextureLoaded_ > 0)
    {
        drawable_->useTexture = true;
    }
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->processMesh(mesh, scene);
    }

    // Then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        this->processNode(node->mChildren[i], scene);
    }
}

void Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    auto geometryRc = new GeometryResource;
    int  triCnt     = 0;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Geometry::Vertice* vertice = new Geometry::Vertice;

        vertice->position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        // TODO: Implement normal map support
        // if (mesh->HasNormals())
        // {
        // vector.x = mesh->mNormals[i].x;
        // vector.y = mesh->mNormals[i].y;
        // vector.z = mesh->mNormals[i].z;
        // vertex.Normal = vector;
        // }

        if (mesh->HasVertexColors(0))
        {
            unsigned char color[] = {
                static_cast<unsigned char>(mesh->mColors[0][i].r * 255),
                static_cast<unsigned char>(mesh->mColors[0][i].g * 255),
                static_cast<unsigned char>(mesh->mColors[0][i].b * 255),
                static_cast<unsigned char>(mesh->mColors[0][i].a * 255)
            };
            colorCpy(vertice->color, color);
        }

        if (mesh->mTextureCoords[0])
        {
            vertice->texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertice->texCoord = glm::vec2(0.0f, 0.0f);
        }

        geometryRc->vertices.push_back(vertice);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face              = mesh->mFaces[i];
        Geometry::Face* geomFace = new Geometry::Face(geometryRc->vertices);

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            geomFace->indices.push_back(face.mIndices[j]);
        }

        geometryRc->faces.push_back(geomFace);
    }

    // Process material
    if (mesh->mMaterialIndex >= 0)
    {
        // TODO: Support other texture map
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        this->loadMaterialTextures(material,
                                   aiTextureType_DIFFUSE, "texture_diffuse", geometryRc);

        // this->loadMaterialTextures(material,
        // aiTextureType_SPECULAR, "texture_specular", geometryRc);

        // this->loadMaterialTextures(material,
        // aiTextureType_AMBIENT, "texture_ambient", geometryRc);
    }

    geometryRcs_.push_back(geometryRc);
}

void Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, GeometryResource* geometryRc)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        string filename = string(str.C_Str());
        filename = directory_ + '/' + filename;

        auto loadedTexture = parentManager_->getTextureResource(filename);

        if (loadedTexture == nullptr)
        {
            // If texture hasn't been loaded already, load it
            loadedTexture = parentManager_->loadTextureResource(filename, typeName, filename);
        }

        if (loadedTexture != nullptr)
        {
            geometryRc->textures.push_back(loadedTexture);
            totalTextureLoaded_++;
        }
    }
}
