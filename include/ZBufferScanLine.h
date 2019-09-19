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

class ZEdge {
public:

    float x;            // Upmost x
    int y;              // Upmost y
    float z;            // Upmost z
    float dx;           // Delta x between scanlines (ending x for horizontal edge)
    int dy;             // Remaining scanlines
    glm::vec2 dtex;     // Delta tex between scanlines
    glm::vec2 texCoord; // Upmost tex
};

typedef vector<ZEdge *> EdgeTable;

class ZPolygon;

typedef ZPolygon                ActivePolygon;
typedef vector<ActivePolygon *> ActivePolygonTable;

class ActiveEdgePair {
public:

    ZEdge *leftEdge  = NULL;
    ZEdge *rightEdge = NULL;
    float z_l;      // Plane depth (left)
    float z_r;      // Plane depth (right)
    float dz_x;     // Plane depth step (x)
    float dz_y;     // Plane depth stop (y)
    glm::vec2 t_l;  // Plane texture (left)
    glm::vec2 t_r;  // Plane texture (right)
    glm::vec2 dt_x; // Plane texture step (x)
    glm::vec2 dt_y; // Plane texture stop (y)
    ZPolygon *polygon = NULL;
};

typedef vector<ActiveEdgePair *> ActiveEdgePairTable;

class ZPolygon {
public:

    ZPolygon()
    {}

    ~ZPolygon()
    {
        for (ZEdge *edge: edges)
        {
            delete edge;
        }
    }

    glm::vec4 depthPlane; // Vertices depth function
    unsigned char color[4];
    int dy;               // Remaining scanlines
    EdgeTable edges;
    EdgeTable unpairedEdges;
    vector<TextureResource *> *textures = NULL;
};

typedef vector<ZPolygon *> PolygonTable;

class ZBufferScanLine {
public:

    ZBufferScanLine(int     width,
                    int     height,
                    GLfloat near,
                    GLfloat far);

    ~ZBufferScanLine();

    // Pipeline
    void reset();

    void draw(GLubyte *buffer);

    void drawLine(int index);

    void drawEdgePair(ActiveEdgePair& edgePair);

    void insertActiveEdgePairs(int            lineIndex,
                               ActivePolygon& polygon);

    // Outer function
    void setMVP(const glm::mat4& MVP)
    {
        _mvp = MVP;
    }

    void setViewDir(const glm::vec3& dir)
    {
        _viewDir = dir;
    }

    void* getLineFrameBuffer()
    {
        return _frameBuffer;
    }

    void insertPolygon(Geometry::Face   *face,
                       GeometryResource *geometry,
                       bool              useTexture);

    int getNumPolygon()
    {
        return _numPolygon;
    }

private:

    // Preparation
    ZEdge* generateEdge(glm::vec3 *p1,
                        glm::vec3 *p2,
                        int      & top,
                        int      & bottom,
                        glm::vec2 *tex1 = NULL,
                        glm::vec2 *tex2 = NULL);

    ActiveEdgePair* generateEdgePair(ZEdge    *left,
                                     ZEdge    *right,
                                     ZPolygon& polygon);

private:

    // OpenGL variables
    int _width;
    int _height;
    int _index;
    GLfloat _near;
    GLfloat _far;
    glm::mat4 _mvp;
    glm::vec3 _viewDir;
    unsigned char _bgColor[4] = { 150, 150, 150, 255 };

    // Buffers
    float *_zBuffer;
    GLubyte *_frameBuffer;
    int _numPolygon;

    // Geometry tables
    vector<PolygonTable>_polygonTables;
    ActivePolygonTable _activePolygonTable;
    ActiveEdgePairTable _activeEdgePairTable;
};
