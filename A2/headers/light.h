#pragma once

#include "common.h"

enum LightType {
	POINT_LIGHT=0,
	DIRECTIONAL_LIGHT=1,
	NUM_LIGHT_TYPES
};

struct Light {
	LightType type;
	Vector3f postORdir;
	Vector3f radiance;

	std::vector<struct Light> load(nlohmann::json sceneConfig);	
};