#pragma once

#include "common.h"
#include "texture.h"

typedef struct Triangles {
    Vector3f aabbmin, aabbmax;
    Vector3f v1, v2, v3;
    Vector3f n1, n2, n3;
    Vector3f centroid;
}Triangles;

typedef struct triBVH triBVH;

typedef struct Surface {
    std::vector<Vector3f> vertices, normals;
    std::vector<Vector3i> indices;
    std::vector<Vector2f> uvs;

    Vector3f aabbmin, aabbmax, aabbcentre;
    Triangles* triangles;
    u_int32_t tricount;
    triBVH* triroot;

    bool isLight;
    uint32_t shapeIdx;

    Vector3f diffuse;
    float alpha;

    Texture diffuseTexture, alphaTexture;

    Interaction rayPlaneIntersect(Ray ray, Vector3f p, Vector3f n);
    Interaction rayTriangleIntersect(Ray ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f n);
    Interaction rayIntersect(Ray ray);
    
    bool checktriaabbinter(Ray& ray, Triangles& triangle);
    bool checktribvhinter(Ray& ray, triBVH* root);
    Interaction aabbRayIntersect(Ray& ray, triBVH* root);


private:
    bool hasDiffuseTexture();
    bool hasAlphaTexture();
}Surface;

typedef struct triBVH {
    triBVH* left;
    triBVH* right;
    Triangles* triangles;
    // Surface* surface;
    Vector3f bvhmin, bvhmax, bvhcentre;
    int count;
}triBVH;

std::vector<Surface> createSurfaces(std::string pathToObj, bool isLight, uint32_t shapeIdx);