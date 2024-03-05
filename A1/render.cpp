#include "render.h"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}

BVH *Integrator::BVHonAABB_root()
{

    // BVH root initialisation
    BVH *root = (BVH *)malloc(sizeof(struct BVH));
    root->left = NULL;
    root->right = NULL;
    root->count = 0;
    root->surface = (Surface *)calloc(sizeof(struct Surface), INT16_MAX);
    root->bvhmax = Vector3f(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    root->bvhmin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    for (auto surface : this->scene.surfaces)
    {
        root->surface[root->count] = surface;
        if (surface.aabbmin.x < root->bvhmin.x)
            root->bvhmin.x = surface.aabbmin.x;
        if (surface.aabbmin.y < root->bvhmin.y)
            root->bvhmin.y = surface.aabbmin.y;
        if (surface.aabbmin.z < root->bvhmin.z)
            root->bvhmin.z = surface.aabbmin.z;

        if (surface.aabbmax.x > root->bvhmax.x)
            root->bvhmax.x = surface.aabbmax.x;
        if (surface.aabbmax.y > root->bvhmax.y)
            root->bvhmax.y = surface.aabbmax.y;
        if (surface.aabbmax.z > root->bvhmax.z)
            root->bvhmax.z = surface.aabbmax.z;

        root->surface[root->count].triroot = triBVH_root(surface);
        // std::cout << "---- Tree ----" << std::endl;
        root->count++;
        // std::cout << "sfdgh" << std::endl;
        // std::cout << root->surface[root->count - 1].triroot->triangles[0].aabbcentre.x << std::endl;
    }
    // std::cout << "Total objects: " << root->count << std::endl;
    root->bvhcentre.x = (root->bvhmin.x + root->bvhmax.x) / 2;
    root->bvhcentre.y = (root->bvhmin.y + root->bvhmax.y) / 2;
    root->bvhcentre.z = (root->bvhmin.z + root->bvhmax.z) / 2;
    // std::cout << "Check 1" << std::endl;
    // std::cout << root->count << std::endl;
    // std::cout << "Check 2" << std::endl;
    if (root->count > 1)
    {
        root = BVHonAABB_init(root);
        return root;
    }
    else
        return root;
}

BVH *Integrator::BVHonAABB_init(BVH *root)
{
    // std::cout << "Hi" << std::endl;
    // BVH tree construction
    root->left = (BVH *)malloc(sizeof(struct BVH));
    root->right = (BVH *)malloc(sizeof(struct BVH));
    BVH *left = root->left;
    BVH *right = root->right;
    left->left = NULL;
    left->right = NULL;
    right->left = NULL;
    right->right = NULL;
    left->count = 0;
    right->count = 0;
    left->surface = (Surface *)calloc(sizeof(struct Surface), INT16_MAX);
    right->surface = (Surface *)calloc(sizeof(struct Surface), INT16_MAX);
    float x = (root->bvhmax.x - root->bvhmin.x);
    float y = (root->bvhmax.y - root->bvhmin.y);
    float z = (root->bvhmax.z - root->bvhmin.z);

    left->bvhmin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    left->bvhmax = Vector3f(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    right->bvhmin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    right->bvhmax = Vector3f(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    int flag = 0;
    if(x <= z && y <= z)
        flag = 2;
    if(x <= y && z <= y)
        flag = 1;

    for (int i = 0; i < root->count; i++)
    {
        if (root->surface[i].aabbcentre.x <= (root->bvhmax[flag] + root->bvhmax[flag])/2)
        {
            left->surface[left->count++] = root->surface[i];
            // std::cout << root->surface[i].aabbmin.x << std::endl;
            if (root->surface[i].aabbmin.x < left->bvhmin.x)
                left->bvhmin.x = root->surface[i].aabbmin.x;
            if (root->surface[i].aabbmin.y < left->bvhmin.y)
                left->bvhmin.y = root->surface[i].aabbmin.y;
            if (root->surface[i].aabbmin.z < left->bvhmin.z)
                left->bvhmin.z = root->surface[i].aabbmin.z;

            if (root->surface[i].aabbmax.x > left->bvhmax.x)
                left->bvhmax.x = root->surface[i].aabbmax.x;
            if (root->surface[i].aabbmax.y > left->bvhmax.y)
                left->bvhmax.y = root->surface[i].aabbmax.y;
            if (root->surface[i].aabbmax.z > left->bvhmax.z)
                left->bvhmax.z = root->surface[i].aabbmax.z;
        }
        else
        {
            right->surface[right->count++] = root->surface[i];
            // printf("def\n");
            if (root->surface[i].aabbmin.x < right->bvhmin.x)
                right->bvhmin.x = root->surface[i].aabbmin.x;
            if (root->surface[i].aabbmin.y < right->bvhmin.y)
                right->bvhmin.y = root->surface[i].aabbmin.y;
            if (root->surface[i].aabbmin.z < right->bvhmin.z)
                right->bvhmin.z = root->surface[i].aabbmin.z;

            if (root->surface[i].aabbmax.x > right->bvhmax.x)
                right->bvhmax.x = root->surface[i].aabbmax.x;
            if (root->surface[i].aabbmax.y > right->bvhmax.y)
                right->bvhmax.y = root->surface[i].aabbmax.y;
            if (root->surface[i].aabbmax.z > right->bvhmax.z)
                right->bvhmax.z = root->surface[i].aabbmax.z;
        }
    }
    // std::cout << "Left: " << left->count << " Right: " << right->count << std::endl;
    left->bvhcentre.x = (left->bvhmin.x + left->bvhmax.x) / 2;
    left->bvhcentre.y = (left->bvhmin.y + left->bvhmax.y) / 2;
    left->bvhcentre.z = (left->bvhmin.z + left->bvhmax.z) / 2;
    right->bvhcentre.x = (right->bvhmin.x + right->bvhmax.x) / 2;
    right->bvhcentre.y = (right->bvhmin.y + right->bvhmax.y) / 2;
    right->bvhcentre.z = (right->bvhmin.z + right->bvhmax.z) / 2;

    if (left->count > 0 && right->count > 0)
    {
        if (left->count > 1)
            left = BVHonAABB_init(left);
        if (right->count > 1)
            right = BVHonAABB_init(right);
    }

    root->left = left;
    root->right = right;
    return root;
}

triBVH *Integrator::triBVH_root(Surface surface)
{
    // std::cout << "hii" << std::endl;
    // BVH root initialisation
    triBVH *root = (triBVH *)malloc(sizeof(struct triBVH));
    root->left = NULL;
    root->right = NULL;
    root->count = 0;
    root->triangles = (Triangles *)calloc(sizeof(struct Triangles), INT16_MAX);
    // std::cout << root->surface->triangles[0].aabbmax.x << std::endl;
    // std::cout << "bc" << std::endl;
    root->bvhmax = Vector3f(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    root->bvhmin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    // std::cout << "In surface: " << surface.tricount << std::endl;
    for (int i = 0; i < surface.tricount; i++)
    {
        // std::cout << surface.triangles[i].v1.x << std::endl;
        // std::cout << "sfdghg" << std::endl; 
        root->triangles[i].v1 = surface.triangles[i].v1;
        root->triangles[i].v2 = surface.triangles[i].v2;
        root->triangles[i].v3 = surface.triangles[i].v3;

        root->triangles[i].n1 = surface.triangles[i].n1;
        root->triangles[i].n2 = surface.triangles[i].n2;
        root->triangles[i].n3 = surface.triangles[i].n3;

        root->triangles[i].centroid = surface.triangles[i].centroid;

        root->triangles[i].aabbmax = surface.triangles[i].aabbmax;
        root->triangles[i].aabbmin = surface.triangles[i].aabbmin;
        
        if (root->triangles[i].aabbmin.x < root->bvhmin.x)
            root->bvhmin.x = root->triangles[i].aabbmin.x;
        if (root->triangles[i].aabbmin.y < root->bvhmin.y)
            root->bvhmin.y = root->triangles[i].aabbmin.y;
        if (root->triangles[i].aabbmin.z < root->bvhmin.z)
            root->bvhmin.z = root->triangles[i].aabbmin.z;

        if (root->triangles[i].aabbmax.x > root->bvhmax.x)
            root->bvhmax.x = root->triangles[i].aabbmax.x;
        if (root->triangles[i].aabbmax.y > root->bvhmax.y)
            root->bvhmax.y = root->triangles[i].aabbmax.y;
        if (root->triangles[i].aabbmax.z > root->bvhmax.z)
            root->bvhmax.z = root->triangles[i].aabbmax.z;
        root->count++;
    }
    // std::cout << root->triangles[0].aabbmax.x << std::endl;
    root->bvhcentre.x = (root->bvhmin.x + root->bvhmax.x) / 2;
    root->bvhcentre.y = (root->bvhmin.y + root->bvhmax.y) / 2;
    root->bvhcentre.z = (root->bvhmin.z + root->bvhmax.z) / 2;
    // std::cout << "Total: " << root->count << std::endl;
    if (root->count > 1)
    {
        root = triBVH_init(root);
        return root;
    }
    else
        return root;
}

triBVH *Integrator::triBVH_init(triBVH *root)
{
    root->left = (triBVH *)malloc(sizeof(struct triBVH));
    root->right = (triBVH *)malloc(sizeof(struct triBVH));
    triBVH *left = root->left;
    triBVH *right = root->right;
    left->left = NULL;
    left->right = NULL;
    right->left = NULL;
    right->right = NULL;
    left->count = 0;
    right->count = 0;
    left->triangles = (Triangles *)calloc(sizeof(struct Triangles), INT16_MAX);
    right->triangles = (Triangles *)calloc(sizeof(struct Triangles), INT16_MAX);
    float x = (root->bvhmax.x - root->bvhmin.x);
    float y = (root->bvhmax.y - root->bvhmin.y);
    float z = (root->bvhmax.z - root->bvhmin.z);

    left->bvhmin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    left->bvhmax = Vector3f(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    right->bvhmin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    right->bvhmax = Vector3f(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    int flag = 0;
    if(x <= y && z <= y)
        flag = 1;
    if(x <= z && y <= z)
        flag = 2;

    for (int i = 0; i < root->count; i++)
    {
        if (root->triangles[i].centroid[flag] <= (root->bvhmax[flag] + root->bvhmax[flag])/2) 
        {
            // std::cout << root->triangles[i].v1.x << std::endl;
            left->triangles[left->count].v1 = root->triangles[i].v1;
            left->triangles[left->count].v2 = root->triangles[i].v2;
            left->triangles[left->count].v3 = root->triangles[i].v3;

            left->triangles[left->count].n1 = root->triangles[i].n1;
            left->triangles[left->count].n2 = root->triangles[i].n2;
            left->triangles[left->count].n3 = root->triangles[i].n3;

            left->triangles[left->count].centroid = root->triangles[i].centroid;
            left->triangles[left->count].aabbmax = root->triangles[i].aabbmax;
            left->triangles[left->count++].aabbmin = root->triangles[i].aabbmin;

            if (root->triangles[i].aabbmin.x < left->bvhmin.x)
                left->bvhmin.x = root->triangles[i].aabbmin.x;
            if (root->triangles[i].aabbmin.y < left->bvhmin.y)
                left->bvhmin.y = root->triangles[i].aabbmin.y;
            if (root->triangles[i].aabbmin.z < left->bvhmin.z)
                left->bvhmin.z = root->triangles[i].aabbmin.z;
            if (root->triangles[i].aabbmax.x > left->bvhmax.x)
                left->bvhmax.x = root->triangles[i].aabbmax.x;
            if (root->triangles[i].aabbmax.y > left->bvhmax.y)
                left->bvhmax.y = root->triangles[i].aabbmax.y;
            if (root->triangles[i].aabbmax.z > left->bvhmax.z)
                left->bvhmax.z = root->triangles[i].aabbmax.z;
        }
        else
        {
            right->triangles[right->count].v1 = root->triangles[i].v1;
            right->triangles[right->count].v2 = root->triangles[i].v2;
            right->triangles[right->count].v3 = root->triangles[i].v3;

            right->triangles[right->count].n1 = root->triangles[i].n1;
            right->triangles[right->count].n2 = root->triangles[i].n2;
            right->triangles[right->count].n3 = root->triangles[i].n3;

            right->triangles[right->count].centroid = root->triangles[i].centroid;
            right->triangles[right->count].aabbmax = root->triangles[i].aabbmax;
            right->triangles[right->count++].aabbmin = root->triangles[i].aabbmin;

            if (root->triangles[i].aabbmin.x < right->bvhmin.x)
                right->bvhmin.x = root->triangles[i].aabbmin.x;
            if (root->triangles[i].aabbmin.y < right->bvhmin.y)
                right->bvhmin.y = root->triangles[i].aabbmin.y;
            if (root->triangles[i].aabbmin.z < right->bvhmin.z)
                right->bvhmin.z = root->triangles[i].aabbmin.z;
            if (root->triangles[i].aabbmax.x > right->bvhmax.x)
                right->bvhmax.x = root->triangles[i].aabbmax.x;
            if (root->triangles[i].aabbmax.y > right->bvhmax.y)
                right->bvhmax.y = root->triangles[i].aabbmax.y;
            if (root->triangles[i].aabbmax.z > right->bvhmax.z)
                right->bvhmax.z = root->triangles[i].aabbmax.z;
        }
    }

    left->bvhcentre.x = (left->bvhmin.x + left->bvhmax.x) / 2;
    left->bvhcentre.y = (left->bvhmin.y + left->bvhmax.y) / 2;
    left->bvhcentre.z = (left->bvhmin.z + left->bvhmax.z) / 2;
    right->bvhcentre.x = (right->bvhmin.x + right->bvhmax.x) / 2;
    right->bvhcentre.y = (right->bvhmin.y + right->bvhmax.y) / 2;
    right->bvhcentre.z = (right->bvhmin.z + right->bvhmax.z) / 2;

    // std::cout << "SB: " << root->bvhmin.x << " " << root->bvhmin.y << " " << root->bvhmin.z << std::endl;
    // std::cout << "BB: " << root->bvhmax.x << " " << root->bvhmax.y << " " << root->bvhmax.z << std::endl;

    // std::cout << "Left: " << left->count << " Right: " << right->count << std::endl;
    if (left->count > 0 && right->count > 0)
    {
        if (left->count > 1)
            left = triBVH_init(left);
        if (right->count > 1)
            right = triBVH_init(right);
    }
    root->left = left;
    root->right = right;
    return root;
}

long long Integrator::render(int option)
{
    BVH * root;
    root = BVHonAABB_root();
    // std::cout << root->count;
    // std::cout << root->surface->triroot->triangles[0].aabbcentre.x << std::endl;

    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++)
    {
        for (int y = 0; y < this->scene.imageResolution.y; y++)
        {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si;
            if (option < 2)
                si = this->scene.rayIntersect(cameraRay, option);
            else
            {
                // std::cout << root->surface[0].triroot->bvhmin.x << std::endl;
                // std::cout << "OOF" << std::endl;
                si = this->scene.rayBVHoutput(cameraRay, option, root);
            }

            if (si.didIntersect)
                this->outputImage.writePixelColor(0.5f * (si.n + Vector3f(1.f, 1.f, 1.f)), x, y);
            else
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: ./render <scene_config> <out_path> <intersection_variant>";
        return 1;
    }
    Scene scene(argv[1]);

    Integrator rayTracer(scene);
    auto renderTime = rayTracer.render(atoi(argv[3]));

    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
