#pragma once

#include "vec.h"

// Forward declaration of BSDF class
class BSDF;

struct Interaction {
    // Position of interaction
    Vector3f p;
    // Normal of the surface at interaction
    Vector3f n;
    // The uv co-ordinates at the intersection point
    Vector2f uv;
    // The viewing direction in local shading frame
    Vector3f wi; 
    // Distance of intersection point from origin of the ray
    float t = 1e30f; 
    // Used for light intersection, holds the radiance emitted by the emitter.
    Vector3f emissiveColor;
    // BSDF at the shading point
    BSDF* bsdf;
    // Vectors defining the orthonormal basis
    Vector3f a, b, c;

    bool didIntersect = false;

    Vector3f toLocal(Vector3f w) {
        // TODO: Implement this
        Vector3f ww;
        ww.x = (this->c.x * w.x) + (this->c.y * w.y) + (this->c.z * w.z);
        ww.y = (this->b.x * w.x) + (this->b.y * w.y) + (this->b.z * w.z);
        ww.z = (this->a.x * w.x) + (this->a.y * w.y) + (this->a.z * w.z);

        return ww;
    }

    Vector3f toWorld(Vector3f w) {
        // TODO: Implement this
        Vector3f wl;
        wl.x = (this->c.x * w.x) + (this->b.x * w.y) + (this->a.x * w.z);
        wl.y = (this->c.y * w.x) + (this->b.y * w.y) + (this->a.y * w.z);
        wl.z = (this->c.z * w.x) + (this->b.z * w.y) + (this->a.z * w.z);

        return wl;
    }
};