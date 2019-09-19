#include "ZBufferScanLine.h"

#include <algorithm>
#include <iostream>
#include <limits>
using namespace std;

#include "HelperTools.h"
#include "ResourceManager.h"
#include "Geometry.h"

const unsigned char errorColor[4] = {
    255, 0, 0, 255
};
static const float  SAME_PIXEL_LIMIT = 0.5f;

// Clip xyz and uv
inline ClipResult clipEdge(glm::vec3      & p1,
                           glm::vec3      & p2,
                           const glm::vec3& p1_o,
                           const glm::vec3& p2_o,
                           glm::vec2      & tex1,
                           glm::vec2      & tex2,
                           ZPolygon        *zPolygon,
                           bool             hasTexture,
                           int              width,
                           int              height)
{
    // Use Cohen-Sutherland clip algorithm
    ClipResult result = CohenSutherlandLineClip(&p1, &p2, width, height);

    if (result != ACCEPTED)
    {
        return result;
    }

    // Interpolate other attributes of the cut edge
    glm::vec2 new1 = tex1;

    if (p1 != p1_o)
    {
        p1.z = computeZ(zPolygon->depthPlane, p1.x, p1.y);
        new1 =
            (tex1 * p1_o.z + glm::length(glm::vec2(p1) - glm::vec2(p1_o)) /
             glm::length(glm::vec2(p2_o) - glm::vec2(p1_o)) *
             (tex2 * p2_o.z - tex1 * p1_o.z))
            / p1.z;
    }

    glm::vec2 new2 = tex2;

    if (p2 != p2_o)
    {
        p2.z = computeZ(zPolygon->depthPlane, p2.x, p2.y);
        new2 =
            (tex1 * p1_o.z + glm::length(glm::vec2(p2) - glm::vec2(p1_o)) /
             glm::length(glm::vec2(p2_o) - glm::vec2(p1_o)) *
             (tex2 * p2_o.z - tex1 * p1_o.z))
            / p2.z;
    }

    tex1 = new1;
    tex2 = new2;

    return result;
}

// TODO: Resolve corner clip problem
inline void generateCorner(glm::vec3& corner, glm::vec2& texCorner, const glm::vec4 *depthPlane,
                           const glm::vec3 *p1, const glm::vec3 *p2, const glm::vec2 *tex1, const glm::vec2 *tex2)
{
    if (p1->y > p2->y)
    {
        SWAP(p1, p2);
    }

    if (p2->y == 0)
    {
        if (p1->x == 0)
        {
            // bottom left
            corner.x = corner.y = 0;
            corner.z = computeZ(*depthPlane, corner.x, corner.y);
        }
        else
        {
            // bottom right
            corner.x = p1->x;
            corner.y = 0;
            corner.z = computeZ(*depthPlane, corner.x, corner.y);
        }
    }
    else
    {
        if (p2->x == 0)
        {
            // top left
            corner.x = 0;
            corner.y = p1->y;
            corner.z = computeZ(*depthPlane, corner.x, corner.y);
        }
        else
        {
            // top right
            corner.x = p2->x;
            corner.y = p1->y;
            corner.z = computeZ(*depthPlane, corner.x, corner.y);
        }
    }
}

// Nearest interpolation
inline void sampleTexture2D(glm::vec2& texCoord, vector<TextureResource *> *textures,
                            unsigned char *dst, const glm::vec3 scale = glm::vec3(1.0f))
{
    if (textures->empty())
    {
        return;
    }

    unsigned char color[4] = {
        0, 0, 0, 0
    };

    for (TextureResource *resource: *textures)
    {
        Geometry::Texture *texture = resource->texture;
        int   width                = texture->width;
        int   height               = texture->height;
        int   channel              = texture->channel;
        float s                    = clipUV(texCoord.s) * (width - 1);
        float t                    = clipUV(texCoord.t) * (height - 1);
        int   u                    = s - floor(s) <
                                     ceil(s) - s ? static_cast<int>(floor(s)) : static_cast<int>(ceil(s));
        int v = height - 1 -
                (t - floor(t) < ceil(t) - t ? static_cast<int>(floor(t)) : static_cast<int>(ceil(t)));

        addColor(color, texture->image + u * channel + v * width * channel);
    }

    divideColor(color, static_cast<int>(textures->size()));

    if (scale == glm::vec3(1.0f))
    {
        fillColor(dst, color);
    }
    else
    {
        fillColor(dst, color, scale);
    }
}

ZBufferScanLine::ZBufferScanLine(int width, int height, GLfloat near, GLfloat far) :
    _width(width), _height(height),
    _near(near), _far(far)
{
    // Allocate tables per scanline
    _polygonTables.resize(height);

    // Allocate buffers for one scanline
    _zBuffer = new float[width];
}

ZBufferScanLine::~ZBufferScanLine()
{
    reset();

    delete[] _zBuffer;
}

void ZBufferScanLine::reset()
{
    // Clear active tables
    _activePolygonTable.clear();
    _activeEdgePairTable.clear();

    // Clear polygons
    // !! Demo behavior, not considering a second rendering
    for (auto itr = _polygonTables.begin(); itr != _polygonTables.end(); ++itr)
    {
        if (itr->empty())
        {
            continue;
        }

        for (ZPolygon *polygon: *itr)
        {
            delete polygon;
        }

        itr->clear();
    }

    _numPolygon = 0;
}

void ZBufferScanLine::draw(GLubyte *buffer)
{
    _frameBuffer = buffer + (_height - 1) * _width * 4;

    for (int i = _height - 1; i >= 0; i--)
    {
        drawLine(i);
        _frameBuffer -= _width * 4;
    }
}

void ZBufferScanLine::drawLine(int index)
{
    _index = index;

    std::fill(_zBuffer,            _zBuffer + _width,            -numeric_limits<float>::max());
    std::fill((int *)_frameBuffer, (int *)_frameBuffer + _width, *((int *)_bgColor));

    // Insert new active polygons
    if (!_polygonTables[index].empty())
    {
        for (ZPolygon *polygon: _polygonTables[index])
        {
            _activePolygonTable.push_back(polygon);
        }
    }

    for (ActivePolygon *polygon: _activePolygonTable)
    {
        // Insert active edge pairs
        insertActiveEdgePairs(index, *polygon);
    }

    // Draw all edge pairs
    for (auto itr = _activeEdgePairTable.begin(); itr != _activeEdgePairTable.end(); ++itr)
    {
        drawEdgePair(**itr);
    }

    for (auto polygon: _activePolygonTable)
    {
        polygon->dy--;
    }

    // Remove finished pairs
    _activeEdgePairTable.erase(std::remove_if(_activeEdgePairTable.begin(), _activeEdgePairTable.end(),
                                              [](ActiveEdgePair *edgePair)
    {
        if ((edgePair->leftEdge->dy <= 0) && (edgePair->rightEdge->dy <= 0))
        {
            delete edgePair;

            return true;
        }

        return false;
    }),
                               _activeEdgePairTable.end());

    // Remove finished polygons
    _activePolygonTable.erase(std::remove_if(_activePolygonTable.begin(), _activePolygonTable.end(),
                                             [](ActivePolygon *polygon)
    {
        return polygon->dy == 0;
    }),
                              _activePolygonTable.end());
}

void ZBufferScanLine::drawEdgePair(ActiveEdgePair& edgePair)
{
    auto& left  = edgePair.leftEdge;
    auto& right = edgePair.rightEdge;

    // Determine edge pair x range (much of the aliasing is caused here)
    int   start_x      = static_cast<int>(left->x);
    int   end_x        = static_cast<int>(left == right ? left->dx : right->x);
    float left_portion = left->x - start_x;

    if ((start_x < 0) || (end_x < 0))
    {
        cout << "bad pair" << endl;

        return;
    }

    // Depth interpolation
    float z_l = edgePair.z_l;
    float z_x = z_l;
    float z_r = z_x + edgePair.dz_x * (end_x - start_x + 1);

    // Texture interpolation
    glm::vec2 t_l  = edgePair.t_l;
    glm::vec2 t_x  = t_l;
    glm::vec2 t_r  = edgePair.t_r;
    glm::vec2 dtex = (t_r * z_r - t_l * z_l) / static_cast<float>(end_x - start_x + 1);

    // Scan along edge pair
    for (int x = start_x; x <= end_x; x++)
    {
        // Determine color
        if (z_x > _zBuffer[x])
        {
            _zBuffer[x] = z_x;

            if (edgePair.polygon->textures != NULL)
            {
                sampleTexture2D(t_x, edgePair.polygon->textures, _frameBuffer + x * 4);
            }
            else
            {
                fillColor(_frameBuffer + x * 4, edgePair.polygon->color);
            }
        }

        // Step interpolation
        float z_x_o = z_x;
        z_x += edgePair.dz_x;

        if (edgePair.polygon->textures != NULL)
        {
            t_x = (t_x * z_x_o + dtex) / z_x;
        }
    }

    // Update pair status
    float z_l_o = edgePair.z_l;
    float z_r_o = edgePair.z_r;
    edgePair.z_l += edgePair.dz_x * left->dx + edgePair.dz_y;
    edgePair.z_r += edgePair.dz_x * right->dx + edgePair.dz_y;

    glm::vec2 t_l_o = edgePair.t_l;
    glm::vec2 t_r_o = edgePair.t_r;
    edgePair.t_l = (edgePair.t_l * z_l_o + left->dtex) / edgePair.z_l;
    edgePair.t_r = (edgePair.t_r * z_r_o + right->dtex) / edgePair.z_r;

    // Update edge status (stall and wait when dy reaching zero)
    if (left->dy > 0)
    {
        left->dy--;
        left->x += left->dx;
    }

    if (right->dy > 0)
    {
        right->dy--;
        right->x += right->dx;
    }

    // if (!edgePair.polygon->resource->texture.empty())
    // edgePair.t_l += edgePair.dt_x * left->dx + edgePair.dt_y;
    // edgePair.z_l += edgePair.dz_y;

    // If left finishes
    if ((left->dy <= 0) && (right->dy >= 0))
    {
        // Pick one line from the unpaired lines
        auto& unPaired = edgePair.polygon->unpairedEdges;

        for (auto itr = unPaired.begin(); itr != unPaired.end(); ++itr)
        {
            if (abs((*itr)->x - left->x) < SAME_PIXEL_LIMIT)
            {
                left         = *(itr);
                edgePair.z_l = left->z;
                edgePair.polygon->unpairedEdges.erase(itr);

                // Roll back one line for the rest line
                right->dy++;
                right->x    -= right->dx;
                edgePair.z_r = z_r_o;
                edgePair.t_r = t_r_o;

                break;
            }
        }
    }

    // If right finishes
    if ((right->dy <= 0) && (left->dy >= 0))
    {
        // Pick one line from the unpaired lines
        auto& unPaired = edgePair.polygon->unpairedEdges;

        for (auto itr = unPaired.begin(); itr != unPaired.end(); ++itr)
        {
            if (abs((*itr)->x - right->x) < SAME_PIXEL_LIMIT)
            {
                right        = *(itr);
                edgePair.z_r = right->z;
                edgePair.polygon->unpairedEdges.erase(itr);

                // Roll back one line for the rest line
                left->dy++;
                left->x     -= left->dx;
                edgePair.z_l = z_l_o;
                edgePair.t_l = t_l_o;

                break;
            }
        }
    }
}

void ZBufferScanLine::insertActiveEdgePairs(int lineIndex, ActivePolygon& polygon)
{
    // Find all edges beginning at current line
    vector<ZEdge *> edgesAtThisLine;

    for (ZEdge *edge: polygon.edges)
    {
        if (edge->y == lineIndex)
        {
            edgesAtThisLine.push_back(edge);
        }
    }

    sort(edgesAtThisLine.begin(), edgesAtThisLine.end(),
         [](ZEdge *a, ZEdge *b)
    {
        return a->x > b->x;
    });

    // Pair edges up
    while (!edgesAtThisLine.empty())
    {
        ZEdge *left  = NULL;
        ZEdge *right = NULL;

        // Pop one edge from list
        left = edgesAtThisLine.back();
        edgesAtThisLine.pop_back();

        float x_left = left->x;

        // Find its pairing edge
        do
        {
            // Find an edge with the same beginning x
            auto iter = edgesAtThisLine.begin();

            while (iter != edgesAtThisLine.end())
            {
                if (abs((*iter)->x - x_left) < 0.5f)
                {
                    right = *iter;
                    edgesAtThisLine.erase(iter);
                    break;
                }
                else
                {
                    ++iter;
                }
            }

            // Not found
            if ((left->dy != 1) && (right == NULL))
            {
                break;
            }

            // Special case: horizontal edge (just discard)
            if ((right != NULL) && (right->dy == 1))
            {
                // Make sure the left edge is horizontal
                SWAP(left, right);
            }

            if (left->dy == 1)
            {
                // Duplicate horizontal edge into a horizontal edge pair
                // _activeEdgePairTable.push_back(generateEdgePair(left, left, polygon));

                // Remove left edge from current pair (as it has been inserted to the horizontal pair)
                // Next pairing edge should start at its ending x
                x_left = left->dx;
                left   = right;
                right  = NULL;
            }

            if (right != NULL)
            {
                break;
            }
        } while (left != NULL);

        // Horizontal edge which has been discarded
        if (left == NULL)
        {
            continue;
        }

        // No pairing edge, Pick one line from the unpaired lines
        if (right == NULL)
        {
            auto& unPaired = polygon.unpairedEdges;

            for (auto itr = unPaired.begin(); itr != unPaired.end(); ++itr)
            {
                if (abs((*itr)->y - left->y) < SAME_PIXEL_LIMIT)
                {
                    right = *(itr);
                    polygon.unpairedEdges.erase(itr);

                    break;
                }
            }
        }

        // Still no pairing edge, preserve for later use
        if (right == NULL)
        {
            polygon.unpairedEdges.push_back(left);
            continue;
        }

        // Make sure leftEdge is on the left
        if ((left->x > right->x + ZERO_LIMIT) || ((abs(left->x - right->x) < ZERO_LIMIT) && (left->dx > right->dx)))
        {
            SWAP(left, right);
        }

        // Insert the edge pair
        _activeEdgePairTable.push_back(generateEdgePair(left, right, polygon));
    }
}

ZEdge * ZBufferScanLine::generateEdge(glm::vec3 *p1,
                                      glm::vec3 *p2,
                                      int      & top,
                                      int      & bottom,
                                      glm::vec2 *tex1,
                                      glm::vec2 *tex2)
{
    // Generate an edge
    ZEdge *zEdge = new ZEdge;

    // Make sure p1 is the upper point
    if (p1->y < p2->y)
    {
        SWAP(p1,   p2);
        SWAP(tex1, tex2);
    }

    // Check for bad edges
    if ((p1->y < 0) || (p1->y > _height - 1) || (p1->x < 0) || (p1->y > _height - 1))
    {
        cout << "Bad edge" << endl;
    }

    if ((p2->y < 0) || (p2->y > _height - 1) || (p2->x < 0) || (p2->y > _height - 1))
    {
        cout << "Bad edge" << endl;
    }

    // Range keeping
    int edgeTop    = static_cast<int>(p1->y);
    int edgeBottom = static_cast<int>(p2->y);

    if (edgeTop > top)
    {
        top = edgeTop;
    }

    if (edgeBottom < bottom)
    {
        bottom = edgeBottom;
    }

    // Insert edge
    zEdge->y  = static_cast<int>(p1->y);
    zEdge->x  = p1->x;
    zEdge->z  = p1->z;
    zEdge->dy = edgeTop - edgeBottom + 1;

    if (tex1 != NULL)
    {
        zEdge->texCoord = *tex1;
        zEdge->dtex     = (*tex2 * p2->z - *tex1 * p1->z) / static_cast<float>(zEdge->dy);
    }

    if (zEdge->dy != 1)
    {
        zEdge->dx = -(p1->x - p2->x) / zEdge->dy;
    }
    else
    {
        // Horizontal edge: label the ending x
        zEdge->dx = p2->x;
    }

    return zEdge;
}

ActiveEdgePair * ZBufferScanLine::generateEdgePair(ZEdge *left, ZEdge *right, ZPolygon& polygon)
{
    ActiveEdgePair *pair = new ActiveEdgePair;

    pair->leftEdge  = left;
    pair->rightEdge = right;

    // depth interpolation
    glm::vec4& depthPlane = polygon.depthPlane;
    pair->z_l  = left->z;
    pair->z_r  = right->z;
    pair->dz_x = depthPlane.z < ZERO_LIMIT ? 0 : -depthPlane.x / depthPlane.z;
    pair->dz_y = depthPlane.z < ZERO_LIMIT ? 0 : depthPlane.y / depthPlane.z;

    // texture interpolation
    if (polygon.textures != NULL)
    {
        pair->t_l = left->texCoord;
        pair->t_r = right->texCoord;
    }

    pair->polygon = &polygon;

    // cout << "________New pair________" << endl <<
    // "left: (" << left->x << ", " << left->y << ", " << left->dx << ", " << left->dy << ", " << endl <<
    // "right: (" << right->x << ", " << right->y << ", " << right->dx << ", " << right->dy << ", " << endl;

    // Insert the edge pair
    return pair;
}

void ZBufferScanLine::insertPolygon(Geometry::Face *face, GeometryResource *geometry, bool useTexture)
{
    vector<TextureResource *> *textures = &geometry->textures;

    // Project points to screen space
    vector<glm::vec3> projected;

    for (int i = 0; i < face->indices.size(); i++)
    {
        glm::vec4 projectedPoint = _mvp * glm::vec4(face->vertices->at(face->indices[i])->position, 1.0f);
        projectedPoint.x = (projectedPoint.x / projectedPoint.w + 0.5f) * (_width - 1);
        projectedPoint.y = (projectedPoint.y / projectedPoint.w + 0.5f) * (_height - 1);
        projectedPoint.z = 1 / projectedPoint.w * 10.0f;

        projected.push_back(projectedPoint);
    }

    // Texture coordinate to use
    vector<glm::vec2> windowTexCoord;

    if (useTexture)
    {
        for (int i = 0; i < face->indices.size(); i++)
        {
            windowTexCoord.push_back(face->vertices->at(face->indices[i])->texCoord);
        }
    }

    // Backface culling
    auto normal = computeNormal(projected[0], projected[1], projected[2]);

    if (glm::dot(normal, glm::vec3(0, 0, 1)) < ZERO_LIMIT)
    {
        return;
    }

    // A little recording
    glm::vec3 firstBegin;
    glm::vec3 lastEnd;
    glm::vec3 thisBegin;
    glm::vec2 thisTex;
    glm::vec2 lastTex;
    glm::vec2 firstTex;
    bool beginned = false;
    int  top      = -1;
    int  bottom   = _height;

    // New ZPolygon
    ZPolygon *zPolygon = new ZPolygon;

    // Calculate depth plane function
    zPolygon->depthPlane = computePlane(normal, projected[0]);

    // Process edges
    for (int i = 0; i < projected.size(); i++)
    {
        int  next = i == projected.size() - 1 ? 0 : i + 1;
        auto p1   = projected[i];
        auto p2   = projected[next];
        glm::vec2 tex1;
        glm::vec2 tex2;

        if (useTexture)
        {
            tex1 = windowTexCoord[i];
            tex2 = windowTexCoord[next];
        }

        // Clip edge to window size
        if (clipEdge(p1, p2, projected[i], projected[next],
                     tex1, tex2, zPolygon, useTexture, _width - 1, _height - 1) == REJECTED)
        {
            continue;
        }

        if (!beginned)
        {
            // The first accepted edge
            firstBegin = p1;

            if (useTexture)
            {
                firstTex = tex1;
            }

            beginned = true;
        }
        else
        {
            // If the edge is clipped, generate a connecting new edge
            thisBegin = p1;

            if (useTexture)
            {
                thisTex = tex1;
            }

            if (glm::length(thisBegin - lastEnd) > SAME_PIXEL_LIMIT)
            {
                zPolygon->edges.push_back(generateEdge(&lastEnd, &thisBegin, top, bottom,
                                                       useTexture ? &lastTex : NULL, useTexture ? &thisTex : NULL));
            }
        }

        lastEnd = p2;

        if (useTexture)
        {
            lastTex = tex2;
        }

        // Insert this edge
        zPolygon->edges.push_back(generateEdge(&p1, &p2, top, bottom,
                                               useTexture ? &tex1 : NULL, useTexture ? &tex2 : NULL));
    }

    // Fix for first and last clipped edges
    if (glm::length(firstBegin - lastEnd) > SAME_PIXEL_LIMIT)
    {
        zPolygon->edges.push_back(generateEdge(&lastEnd, &firstBegin, top, bottom,
                                               useTexture ? &lastTex : NULL, useTexture ? &firstTex : NULL));
    }

    // If nothing is inserted, cull this polygon out
    if ((top == -1) || (bottom == _height))
    {
        delete zPolygon;

        return;
    }

    // Insert polygon
    fillColor(zPolygon->color, face->vertices->at(face->indices[0])->color, true);
    zPolygon->dy = top - bottom + 1;

    if (useTexture)
    {
        zPolygon->textures = textures;
    }

    _polygonTables[top].push_back(zPolygon);
    _numPolygon++;
}
