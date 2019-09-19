#pragma once

#include <glm/glm.hpp>
#include <SOIL\SOIL.h>

#include <vector>

inline void colorDiv(unsigned char dst[3], int num)
{
    dst[2] = dst[2] / num; // R -> B
    dst[1] = dst[1] / num; // G -> G
    dst[0] = dst[0] / num; // B -> R
}

inline void colorAdd(unsigned char dst[3], const unsigned char src[3])
{
    dst[2] += src[0]; // R -> B
    dst[1] += src[1]; // G -> G
    dst[0] += src[2]; // B -> R
}

inline void colorMix(unsigned char dst[3], const unsigned char src[3], float scale)
{
    dst[2] += static_cast<unsigned int>(src[0] * scale); // R -> B
    dst[1] += static_cast<unsigned int>(src[1] * scale); // G -> G
    dst[0] += static_cast<unsigned int>(src[2] * scale); // B -> R
}

inline void colorCpy(unsigned char* dst, const unsigned char* src, bool flip = false, bool alpha = false)
{
    if (flip)
    {
        dst[2] = src[0]; // R -> B
        dst[1] = src[1]; // G -> G
        dst[0] = src[2]; // B -> R
    }
    else
    {
        dst[0] = src[0]; // R -> R
        dst[1] = src[1]; // G -> G
        dst[2] = src[2]; // B -> B
    }

    if (alpha)
    {
        dst[3] = src[3]; // Alpha
    }
}

inline void colorCpy(unsigned char      * dst,
                     const unsigned char* src,
                     const glm::vec3    & scale,
                     bool                 flip  = false,
                     bool                 alpha = false)
{
    if (flip)
    {
        dst[2] = static_cast<unsigned int>(scale.r * src[0]); // R -> B
        dst[1] = static_cast<unsigned int>(scale.g * src[1]); // G -> G
        dst[0] = static_cast<unsigned int>(scale.b * src[2]); // B -> R
    }
    else
    {
        dst[0] = static_cast<unsigned int>(scale.r * src[0]); // R -> R
        dst[1] = static_cast<unsigned int>(scale.g * src[1]); // G -> G
        dst[2] = static_cast<unsigned int>(scale.b * src[2]); // B -> B
    }

    if (alpha)
    {
        dst[3] = src[3]; // Alpha
    }
}

inline glm::vec4 colorToVec(unsigned char src[4])
{
    return glm::vec4(src[0], src[1], src[2], src[3]);
}

// Danger! Delete this pointer after you get it!
inline unsigned char* vecToColor(const glm::vec4& fcolor)
{
    unsigned char* color = new unsigned char[4];

    for (int i = 0; i < 4; i++)
    {
        color[i] = static_cast<unsigned int>(fcolor[i]);
    }

    return color;
}

namespace Geometry {
struct Vertice {
    Vertice()
    {}

    Vertice(const glm::vec3& position, glm::vec4 color = glm::vec4(255, 255, 255, 255)) :
        position(position)
    {
        this->color[0] = static_cast<unsigned char>(color.r);
        this->color[1] = static_cast<unsigned char>(color.g);
        this->color[2] = static_cast<unsigned char>(color.b);
        this->color[3] = static_cast<unsigned char>(color.a);
    }

    Vertice(const glm::vec3& position, const glm::vec2& texCoord, glm::vec4 color = glm::vec4(255, 255, 255, 255)) :
        position(position),
        texCoord(texCoord)
    {
        this->color[0] = static_cast<unsigned char>(color.r);
        this->color[1] = static_cast<unsigned char>(color.g);
        this->color[2] = static_cast<unsigned char>(color.b);
        this->color[3] = static_cast<unsigned char>(color.a);
    }

    unsigned char color[4] = { 0, 0, 0, 255 };
    glm::vec3     position;
    glm::vec2     texCoord;
};

struct Face {
    Face(const std::vector<Vertice *>& vertices) :
        vertices(vertices)
    {}

    std::vector<Vertice *>vertices;
    std::vector<int>      indices;
};

struct Texture {
    Texture(unsigned char* image, int width, int height, int channel               = 4, int type= 0) :
        image(image), height(height), width(width), channel(channel)
    {
        size = height * width * channel;
    }

    ~Texture()
    {
        SOIL_free_image_data(image);
    }

    unsigned char* image;
    int            width;
    int            height;
    int            channel;
    int            size;
};
}
