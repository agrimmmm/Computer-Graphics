#include "scene.h"

Vector3f camFrom;

Scene::Scene(std::string sceneDirectory, std::string sceneJson)
{
    nlohmann::json sceneConfig;
    try {
        sceneConfig = nlohmann::json::parse(sceneJson);
    }
    catch (std::runtime_error e) {
        std::cerr << "Could not parse json." << std::endl;
        exit(1);
    }

    this->parse(sceneDirectory, sceneConfig);
}

Scene::Scene(std::string pathToJson)
{
    std::string sceneDirectory;

#ifdef _WIN32
    const size_t last_slash_idx = pathToJson.rfind('\\');
#else
    const size_t last_slash_idx = pathToJson.rfind('/');
#endif

    if (std::string::npos != last_slash_idx) {
        sceneDirectory = pathToJson.substr(0, last_slash_idx);
    }

    nlohmann::json sceneConfig;
    try {
        std::ifstream sceneStream(pathToJson.c_str());
        sceneStream >> sceneConfig;
    }
    catch (std::runtime_error e) {
        std::cerr << "Could not load scene .json file." << std::endl;
        exit(1);
    }

    this->parse(sceneDirectory, sceneConfig);
}

void Scene::parse(std::string sceneDirectory, nlohmann::json sceneConfig)
{
    // Output
    try {
        auto res = sceneConfig["output"]["resolution"];
        this->imageResolution = Vector2i(res[0], res[1]);
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "\"output\" field with resolution, filename & spp should be defined in the scene file." << std::endl;
        exit(1);
    }

    // Cameras
    try {
        auto cam = sceneConfig["camera"];

        this->camera = Camera(
            Vector3f(cam["from"][0], cam["from"][1], cam["from"][2]),
            Vector3f(cam["to"][0], cam["to"][1], cam["to"][2]),
            Vector3f(cam["up"][0], cam["up"][1], cam["up"][2]),
            float(cam["fieldOfView"]),
            this->imageResolution
        );

        camFrom = Vector3f(this->camera.from.x, this->camera.from.y, this->camera.from.z);
        this->camera.to -= this->camera.from;
        this->camera.upperLeft -= this->camera.from;
        this->camera.from -= this->camera.from;
    }
    catch (nlohmann::json::exception e) {
        std::cerr << "No camera(s) defined. Atleast one camera should be defined." << std::endl;
        exit(1);
    }

    // Surface
    try {
        auto surfacePaths = sceneConfig["surface"];

        uint32_t surfaceIdx = 0;
        for (std::string surfacePath : surfacePaths) {
            surfacePath = sceneDirectory + "/" + surfacePath;

            auto surf = createSurfaces(surfacePath, /*isLight=*/false, /*idx=*/surfaceIdx);
            this->surfaces.insert(this->surfaces.end(), surf.begin(), surf.end());

            surfaceIdx = surfaceIdx + surf.size();
        }
    }
    catch (nlohmann::json::exception e) {
        std::cout << "No surfaces defined." << std::endl;
    }
    // this->camera.from -= this->camera.from;

}

bool Scene::RayIntersectAABB(Ray& ray, Surface& surface)
{
    float tx1 = (surface.aabbmin.x - ray.o.x) / ray.d.x, tx2 = (surface.aabbmax.x - ray.o.x) / ray.d.x;
    float tmin = std::min( tx1, tx2 ), tmax = std::max( tx1, tx2 );
    float ty1 = (surface.aabbmin.y - ray.o.y) / ray.d.y, ty2 = (surface.aabbmax.y - ray.o.y) / ray.d.y;
    tmin = std::max( tmin, std::min( ty1, ty2 ) ), tmax = std::min( tmax, std::max( ty1, ty2 ) );
    float tz1 = (surface.aabbmin.z - ray.o.z) / ray.d.z, tz2 = (surface.aabbmax.z - ray.o.z) / ray.d.z;
    tmin = std::max( tmin, std::min( tz1, tz2 ) ), tmax = std::min( tmax, std::max( tz1, tz2 ) );
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

bool Scene::RayIntersectBVH(Ray& ray, BVH* root)
{
    float tx1 = (root->bvhmin.x - ray.o.x) / ray.d.x, tx2 = (root->bvhmax.x - ray.o.x) / ray.d.x;
    float tmin = std::min( tx1, tx2 ), tmax = std::max( tx1, tx2 );
    float ty1 = (root->bvhmin.y - ray.o.y) / ray.d.y, ty2 = (root->bvhmax.y - ray.o.y) / ray.d.y;
    tmin = std::max( tmin, std::min( ty1, ty2 ) ), tmax = std::min( tmax, std::max( ty1, ty2 ) );
    float tz1 = (root->bvhmin.z - ray.o.z) / ray.d.z, tz2 = (root->bvhmax.z - ray.o.z) / ray.d.z;
    tmin = std::max( tmin, std::min( tz1, tz2 ) ), tmax = std::min( tmax, std::max( tz1, tz2 ) );
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

Interaction Scene::rayBVHoutput(Ray& ray, int option, BVH* root)
{
    if(option > 3)
    {
        std::cerr << "Invalid option" << std::endl;
        exit(1);
    }

    Interaction siFinal;

    // std::cout << "Hola" << std::endl;
    if(!RayIntersectBVH(ray, root))
        return siFinal;
    if(root->left == NULL && root->right == NULL)
    {
        for(int i=0; i<root->count; i++)
        {
            if(!RayIntersectAABB(ray, root->surface[i]))
                continue;
            Interaction si;
            if(option == 3)
                si = root->surface[i].aabbRayIntersect(ray, root->surface[i].triroot);
            else
                si = root->surface[i].rayIntersect(ray);
            if(si.t <= ray.t)
            {
                siFinal = si;
                ray.t = si.t;
            }
        }
        // std::cout << "rootisleaf" << std::endl;
        return siFinal;
    }
    else{
        Interaction l = rayBVHoutput(ray, option, root->left);
        Interaction r = rayBVHoutput(ray, option, root->right);
        if(l.t <= r.t)
            return l;
        else
            return r;
    } 
        // return rayBVHnodes(ray, option, root);
}

Interaction Scene::rayIntersect(Ray& ray, int option)
{
    Interaction siFinal;

    for (auto& surface : this->surfaces) {
        if(option == 1)
        {
            if(!RayIntersectAABB(ray, surface))
                continue;
        }

        Interaction si = surface.rayIntersect(ray);
        if (si.t <= ray.t) {    
            siFinal = si;
            ray.t = si.t;
        }
    }

    return siFinal;
}