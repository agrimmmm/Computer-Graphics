#pragma once

#include "scene.h"

struct Integrator {
    Integrator(Scene& scene);

    long long render(int option);

    BVH* BVHonAABB_root();
    BVH* BVHonAABB_init(BVH* root);

    triBVH* triBVH_root(Surface surface);
    triBVH* triBVH_init(triBVH* root);

    Scene scene;
    Texture outputImage;
};