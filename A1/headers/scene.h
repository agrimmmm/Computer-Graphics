#pragma once

#include "camera.h"
#include "surface.h"

typedef struct BVH {
    BVH* left;
    BVH* right;
    Surface* surface;
    Vector3f bvhmin, bvhmax, bvhcentre;
    int count;
}BVH;


struct Scene {
    std::vector<Surface> surfaces;
    Camera camera;
    Vector2i imageResolution;

    Scene() {};
    Scene(std::string sceneDirectory, std::string sceneJson);
    Scene(std::string pathToJson);
    
    void parse(std::string sceneDirectory, nlohmann::json sceneConfig);

    bool RayIntersectAABB(Ray& ray, Surface& surface);
    bool RayIntersectBVH(Ray& ray, BVH* root);
    Interaction rayBVHoutput(Ray& ray, int option, BVH* root);
    Interaction rayIntersect(Ray& ray, int option);
};