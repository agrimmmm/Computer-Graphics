#pragma once

#include "common.h"
#include "texture.h"

struct Tri {
    Vector3f v1, v2, v3;
    Vector2f uv1, uv2, uv3;
    Vector3f normal;
    Vector3f centroid;
    
    AABB bbox;
};

struct Surface {
    std::vector<Vector3f> vertices, normals;
    std::vector<Vector3i> indices;
    std::vector<Vector2f> uvs;

    BVHNode* nodes;
    int numBVHNodes = 0;

    std::vector<Tri> tris;
    std::vector<uint32_t> triIdxs;
    AABB bbox;

    bool isLight;
    uint32_t shapeIdx;

    Vector3f diffuse;
    float alpha;

    Texture diffuseTexture, alphaTexture;

    void buildBVH();
    uint32_t getIdx(uint32_t idx);
    void updateNodeBounds(uint32_t nodeIdx);
    void subdivideNode(uint32_t nodeIdx);
    void intersectBVH(uint32_t nodeIdx, Ray& ray, Interaction& si);
    float TriangleArea(Vector3f v1, Vector3f v2, Vector3f v3);

    Interaction rayPlaneIntersect(Ray ray, Vector3f p, Vector3f n);
    Interaction rayTriangleIntersect(Ray ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f n);
    Interaction rayIntersect(Ray& ray);

    bool hasDiffuseTexture();
    bool hasAlphaTexture();
};

std::vector<Surface> createSurfaces(std::string pathToObj, bool isLight, uint32_t shapeIdx);