#pragma once

#include "scene.h"

struct Integrator {
    Integrator(Scene& scene);

    long long render(int option);

    Scene scene;
    Texture outputImage;
    std::vector<Light> lights;
};