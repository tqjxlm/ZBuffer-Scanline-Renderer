#pragma once

#include <vector>
using namespace std;

static const float cubeVertices[6][4][3] = {
    // back face
    1.0f,  1.0f,   -1.0f,
    1.0f,  -1.0f,  -1.0f,
    -1.0f, -1.0f,  -1.0f,
    -1.0f, 1.0f,   -1.0f,

    // front face
    -1.0f, -1.0f,  1.0f,
    1.0f,  -1.0f,  1.0f,
    1.0f,  1.0f,   1.0f,
    -1.0f, 1.0f,   1.0f,

    // left face
    -1.0f, 1.0f,   -1.0f,
    -1.0f, -1.0f,  -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, 1.0f,   1.0f,

    // right face
    1.0f,  1.0f,   1.0f,
    1.0f,  -1.0f,  1.0f,
    1.0f,  -1.0f,  -1.0f,
    1.0f,  1.0f,   -1.0f,

    // bottom face
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  -1.0f,
    1.0f,  -1.0f,  -1.0f,
    1.0f,  -1.0f,  1.0f,

    // top face
    -1.0f, 1.0f,   -1.0f,
    -1.0f, 1.0f,   1.0f,
    1.0f,  1.0f,   1.0f,
    1.0f,  1.0f,   -1.0f,
};
static const unsigned char cubeColors[6][4] = {
    255, 255,   0,       255,
    255, 0,     255,     255,
    0,   255,   255,     255,
    255, 0,     0,       255,
    0,   255,   0,       255,
    0,   0,     255,     255
};
static const float quadVertices[4][3] = {
    -1.0f, -1.0f,  0.0f,
    1.0f,  -1.0f,  0.0f,
    1.0f,  1.0f,   0.0f,
    -1.0f, 1.0f,   0.0f
};
static const float triangleVertices[3][3] = {
    0.0f,  0.5f,   0.0f,
    -0.5f, -0.5f,  0.0f,
    0.5f,  -0.5f,  0.0f
};
static const float texCoords[4][2] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};
vector<int> quadIndice = { 0, 1, 2, 3 };
vector<int> triIndice  = { 0, 1, 2 };
