#include "light.h"

Light::Light(LightType type, nlohmann::json config) {
    switch (type) {
        case LightType::POINT_LIGHT:
            this->position = Vector3f(config["location"][0], config["location"][1], config["location"][2]);
            break;
        case LightType::DIRECTIONAL_LIGHT:
            this->direction = Vector3f(config["direction"][0], config["direction"][1], config["direction"][2]);
            break;
        case LightType::AREA_LIGHT:
            this->center = Vector3f(config["center"][0], config["center"][1], config["center"][2]);
            this->vx = Vector3f(config["vx"][0], config["vx"][1], config["vx"][2]);
            this->vy = Vector3f(config["vy"][0], config["vy"][1], config["vy"][2]);
            this->normal = Vector3f(config["normal"][0], config["normal"][1], config["normal"][2]);
            break;
        default:
            std::cout << "WARNING: Invalid light type detected";
            break;
    }

    this->radiance = Vector3f(config["radiance"][0], config["radiance"][1], config["radiance"][2]);
    this->type = type;
}

std::pair<Vector3f, LightSample> Light::sample(Interaction *si) {
    LightSample ls;
    memset(&ls, 0, sizeof(ls));

    Vector3f radiance;
    switch (type) {
        case LightType::POINT_LIGHT:
            ls.wo = (position - si->p);
            ls.d = ls.wo.Length();
            ls.wo = Normalize(ls.wo);
            radiance = (1.f / (ls.d * ls.d)) * this->radiance;
            break;
        case LightType::DIRECTIONAL_LIGHT:
            ls.wo = Normalize(direction);
            ls.d = 1e10;
            radiance = this->radiance;
            break;
        case LightType::AREA_LIGHT:
            // TODO: Implement this
            float u = next_float();
            float v = next_float();

            Vector3f x = this->center - this->vx - this->vy;
            x += (2 * u * this->vx) + (2 * v * this->vy);
            ls.wo = x - si->p;
            ls.d = (x - si->p).Length();
            ls.wo = Normalize(ls.wo);

            radiance = Vector3f(0, 0, 0);
            float costheta_l = Dot(-ls.wo, this->normal) / (ls.wo.Length() * this->normal.Length());
            if(costheta_l < 0.f)
                radiance = Vector3f(0, 0, 0);
            else
            
                radiance = this->radiance * 4 * (this->vx.Length() * this->vy.Length()) * costheta_l / (ls.d * ls.d);
            break;
    }
    return { radiance, ls };
}

Interaction Light::intersectLight(Ray *ray) {
    Interaction si;
    memset(&si, 0, sizeof(si));

    if (type == LightType::AREA_LIGHT) {
        //Check Plane Intersection
        float dDotN = Dot(ray->d, this->normal);
        if (dDotN != 0.f) {
            float t = -Dot((ray->o - this->center), this->normal) / dDotN;

            // std::cout << "hi" << std::endl;
            if (t >= 0.f) {
                //Check Rectangle Intersection
                Vector3f p = ray->o + t * ray->d;
                Vector3f op = p - this->center;
                float xcomp = Dot(this->vx, op) / vx.Length();
                float ycomp = Dot(this->vy, op) / vy.Length();
                if(xcomp < 0.f)
                    xcomp = -xcomp;
                if(ycomp < 0.f)
                    ycomp = -ycomp;

                if(Dot(ray->d, this->normal) > 0)
                    return si;

                // std::cout << "Xcomp: " << xcomp << std::endl;
                // std::cout << "Ycomp: " << ycomp << std::endl;
                if(xcomp <= vx.Length() && ycomp <= vy.Length()) {
                    si.didIntersect = true;
                    si.p = p;
                    si.n = this->normal;
                    si.t = t;
                    si.emissiveColor = this->radiance;
                }
            }
        }
    }

    return si;
}