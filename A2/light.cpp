#include "light.h"

std::vector<struct Light> Light::load(nlohmann::json sceneConfig)
{
    std::vector<struct Light> lights;

    try {
        auto dirLights = sceneConfig["directionalLights"];

        for(auto& dirLight : dirLights)
        {
            this->type = DIRECTIONAL_LIGHT;
            this->postORdir = Vector3f(dirLight["direction"][0], dirLight["direction"][1], dirLight["direction"][2]);
            this->radiance = Vector3f(dirLight["radiance"][0], dirLight["radiance"][1], dirLight["radiance"][2]);
            lights.push_back(*this);
        }

        auto pointLights = sceneConfig["pointLights"];

        for(auto& pointLight : pointLights)
        {
            this->type = POINT_LIGHT;
            this->postORdir = Vector3f(pointLight["location"][0], pointLight["location"][1], pointLight["location"][2]);
            this->radiance = Vector3f(pointLight["radiance"][0], pointLight["radiance"][1], pointLight["radiance"][2]);
            lights.push_back(*this);
        }
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "No lights defined. Atleast one light should be defined." << std::endl;
        exit(1);
    }
    
    return lights;
}