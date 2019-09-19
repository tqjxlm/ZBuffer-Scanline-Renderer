#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <vector>
#include <set>
using namespace std;

#include "Geometry.h"

class GeometryResource;
class TextureResource;
class DrawableObject;

struct ZEdge {
    float     x;        // Upmost x
    int       y;        // Upmost y
    float     z;        // Upmost z
    float     dx;       // Delta x between scanlines (ending x for horizontal edge)
    int       dy;       // Remaining scanlines
    glm::vec2 dtex;     // Delta tex between scanlines
    glm::vec2 texCoord; // Upmost tex
};
typedef vector<ZEdge *> EdgeTable;

struct ZPolygon {
    ZPolygon()
    {}

    ~ZPolygon()
    {
        for (ZEdge* edge: edges)
        {
            delete edge;
        }
    }

    glm::vec4                  depthPlane; // Vertices depth function
    unsigned char              color[4];
    int                        dy;         // Remaining scanlines
    EdgeTable                  edges;
    EdgeTable                  unpairedEdges;
    vector<TextureResource *>* textures = nullptr;
};
typedef vector<ZPolygon *> PolygonTable;

struct ActiveEdgePair {
    ZEdge   * leftEdge  = nullptr;
    ZEdge   * rightEdge = nullptr;
    float     z_l;  // Plane depth (left)
    float     z_r;  // Plane depth (right)
    float     dz_x; // Plane depth step (x)
    float     dz_y; // Plane depth stop (y)
    glm::vec2 t_l;  // Plane texture (left)
    glm::vec2 t_r;  // Plane texture (right)
    glm::vec2 dt_x; // Plane texture step (x)
    glm::vec2 dt_y; // Plane texture stop (y)
    ZPolygon* polygon = nullptr;
};
typedef vector<ActiveEdgePair *> ActiveEdgePairTable;

class ZBufferScanLine {
public:

    ZBufferScanLine(int     width,
                    int     height,
                    GLfloat near,
                    GLfloat far);

    ~ZBufferScanLine();

    // Pipeline
    void reset();

    void draw(GLubyte* buffer);

    void drawLine(int index);

    void drawEdgePair(ActiveEdgePair& edgePair);

    void insertActiveEdgePairs(int       lineIndex,
                               ZPolygon& polygon);

    // Outer function
    void setMVP(const glm::mat4& MVP)
    {
        mvp_ = MVP;
    }

    void setViewDir(const glm::vec3& dir)
    {
        viewDir_ = dir;
    }

    void* getLineFrameBuffer()
    {
        return frameBuffer_;
    }

    void insertPolygon(Geometry::Face  * face,
                       GeometryResource* geometry,
                       bool              useTexture);

    int getNumPolygon()
    {
        return numPolygon_;
    }

private:

    // Preparation
    ZEdge* generateEdge(glm::vec3* p1,
                        glm::vec3* p2,
                        int      & top,
                        int      & bottom,
                        bool       useTexture,
                        glm::vec2  tex1,
                        glm::vec2  tex2);

    ActiveEdgePair* generateEdgePair(ZEdge   * left,
                                     ZEdge   * right,
                                     ZPolygon& polygon);

private:

    // OpenGL variables
    int width_;
    int height_;
    GLfloat near_;
    GLfloat far_;
    glm::mat4 mvp_;
    glm::vec3 viewDir_;
    unsigned char bgColor_[4] = { 150, 150, 150, 255 };

    // Buffers
    float* zBuffer_;
    GLubyte* frameBuffer_;
    int numPolygon_;

    // Geometry tables
    vector<PolygonTable>polygonTables_;
    PolygonTable activePolygonTable_;
    ActiveEdgePairTable activeEdgePairTable_;
};
