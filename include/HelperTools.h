#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define SWAP(a, b) { auto tmp = a; a = b; b = tmp; }
#define CLEARZ(a) glm::vec3(a.x, a.y, 0)

static const float FLT_EPS = 1e-3f;

inline glm::vec3 computeNormal(glm::vec3 const& a, glm::vec3 const& b, glm::vec3 const& c)
{
    return glm::normalize(glm::cross(c - b, a - b));
}

inline glm::vec4 computePlane(glm::vec3 const& normal, glm::vec3 const& point)
{
    return glm::vec4(normal.x, normal.y, normal.z, -glm::dot(normal, point));
}

inline float computeZ(glm::vec4 const& plane, float x, float y)
{
    if (abs(plane.z) < FLT_EPS)
    {
        return 0; // ? resolve outside
    }

    return -(plane.x * x + plane.y * y + plane.w) / plane.z;
}

inline float computeY(glm::vec4 const& plane, float x, float z)
{
    if (abs(plane.y) < FLT_EPS)
    {
        return 0; // ? resolve outside
    }

    return -(plane.x * x + plane.z * z + plane.w) / plane.y;
}

inline float computeX(glm::vec4 const& plane, float y, float z)
{
    if (abs(plane.x) < FLT_EPS)
    {
        return 0; // ? resolve outside
    }

    return -(plane.z * z + plane.y * y + plane.w) / plane.x;
}

enum ClipResult
{
    REJECTED = false,
    ACCEPTED = true
};

typedef int OutCode;

static const int INSIDE = 0; // 0000
static const int LEFT   = 1; // 0001
static const int RIGHT  = 2; // 0010
static const int BOTTOM = 4; // 0100
static const int TOP    = 8; // 1000

// Compute the bit code for a point (x, y) using the clip rectangle
inline OutCode computeOutCode(double x, double y, int width, int height)
{
    OutCode code;

    code = INSIDE; // initialised as being inside of [[clip window]]

    if (x < 0)     // to the left of clip window
    {
        code |= LEFT;
    }
    else if (x > width - 1) // to the right of clip window
    {
        code |= RIGHT;
    }

    if (y < 0) // below the clip window
    {
        code |= BOTTOM;
    }
    else if (y > height - 1) // above the clip window
    {
        code |= TOP;
    }

    return code;
}

// Cohen¨CSutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
// diagonal from (xmin, ymin) to (xmax, ymax).
inline ClipResult CohenSutherlandLineClip(glm::vec3* p1, glm::vec3* p2, int width, int height)
{
    float x0 = p1->x;
    float x1 = p2->x;
    float y0 = p1->y;
    float y1 = p2->y;

    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    OutCode outcode0 = computeOutCode(x0, y0, width, height);
    OutCode outcode1 = computeOutCode(x1, y1, width, height);

    while (true)
    {
        if (!(outcode0 | outcode1)) // Bitwise OR is 0. Trivially accept and get out of loop
        {
            p1->x = x0;
            p1->y = y0;
            p2->x = x1;
            p2->y = y1;

            return ACCEPTED;
        }
        else if (outcode0 & outcode1) // Bitwise AND is not 0. (implies both end points are in the same region outside the window). Reject and get out of loop
        {
            return REJECTED;
        }
        else
        {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            float x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

            // Now find the intersection point;
            // use formulas:
            // slope = (y1 - y0) / (x1 - x0)
            // x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
            // y = y0 + slope * (xm - x0), where xm is xmin or xmax
            if (outcodeOut & TOP) // point is above the clip rectangle
            {
                x = x0 + (x1 - x0) * (height - 1 - y0) / (y1 - y0);
                y = static_cast<float>(height - 1);
            }
            else if (outcodeOut & BOTTOM) // point is below the clip rectangle
            {
                x = x0 + (x1 - x0) * (0 - y0) / (y1 - y0);
                y = 0;
            }
            else if (outcodeOut & RIGHT) // point is to the right of clip rectangle
            {
                y = y0 + (y1 - y0) * (width - 1 - x0) / (x1 - x0);
                x = static_cast<float>(width - 1);
            }
            else if (outcodeOut & LEFT) // point is to the left of clip rectangle
            {
                y = y0 + (y1 - y0) * (0 - x0) / (x1 - x0);
                x = 0;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0)
            {
                x0       = x;
                y0       = y;
                outcode0 = computeOutCode(x0, y0, width, height);
            }
            else
            {
                x1       = x;
                y1       = y;
                outcode1 = computeOutCode(x1, y1, width, height);
            }
        }
    }
}

inline void clipUV(int& u, int& v, int width, int height)
{
    if (u < 0)
    {
        u = width - 1 + u;
    }

    if (u > width - 1)
    {
        u = u - width;
    }

    if (v < 0)
    {
        v = height - 1 + v;
    }

    if (v > width - 1)
    {
        v = v - height;
    }
}

inline float clipUV(float s)
{
    while (s < 0)
    {
        s += 1;
    }

    while (s > 1)
    {
        s -= 1;
    }

    return s;
}
