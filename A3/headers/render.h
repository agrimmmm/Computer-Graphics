#pragma once

#include "scene.h"

struct Integrator {
    Integrator(Scene& scene);

    Vector3f uniformHemisphereSampling(Interaction si);
    Vector3f CosineWeightedSampling(Interaction si);
    long long render(int spp, int sampling_mode);

    Scene scene;
    Texture outputImage;
};