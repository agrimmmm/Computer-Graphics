#include "render.h"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
    this->lights = this->scene.Lights;
}

long long Integrator::render(int option)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            Vector3f dirRadiance = Vector3f(0.f, 0.f, 0.f);
            Vector3f ptRadiance = Vector3f(0.f, 0.f, 0.f);
            Vector3f Radiance = Vector3f(0.f, 0.f, 0.f);
            
            Texture texture = si.tex;
            Vector3f cx;
            if(si.tex.resolution.x == 0 || si.tex.resolution.y == 0)
                cx = si.diffuse;
            else
            {
                if(option == 0)
                    cx = texture.nearestNeighbourFetch(si.uv, si.tex.resolution);
                else if(option == 1)
                    cx = texture.bilinearFetch(si.uv, si.tex.resolution);
                else
                {
                    std::cout << "Invalid option" << std::endl;
                    return -1;
                }
            } 
            // std::cout << cx.x << " " << cx.y << " " << cx.z << std::endl;

            for(auto& light : this->lights)
            {
                if(light.type == DIRECTIONAL_LIGHT)
                {
                    Ray shadowRay  = Ray(si.p + si.n*1e-5, light.postORdir);
                    Interaction shadowIntersect = this->scene.rayIntersect(shadowRay);
                    float costheta = Dot(light.postORdir, si.n);
                    // if(costheta < 0)
                    //     shadowIntersect.didIntersect = false;
                    if(shadowIntersect.didIntersect)
                        continue;

                    dirRadiance += light.radiance * cx * costheta/M_PI;
                }
                else if(light.type == POINT_LIGHT)
                {
                    float r = (si.p - light.postORdir).Length();
                    Ray shadowRay  = Ray(si.p + si.n*1e-5, Normalize(light.postORdir - si.p));
                    Interaction shadowIntersect = this->scene.rayIntersect(shadowRay);
                    float costheta = Dot(Normalize(light.postORdir - si.p), si.n);
                    // if(costheta < 0)
                    //     shadowIntersect.didIntersect = false;
                    if(shadowIntersect.didIntersect && (shadowIntersect.t - r) < 1e-3)
                        continue;


                    ptRadiance += light.radiance * cx * costheta/(r * r * M_PI);
                }
            }
            Radiance = dirRadiance + ptRadiance;

            if (si.didIntersect)
                this->outputImage.writePixelColor(Radiance, x, y);
            else
                this->outputImage.writePixelColor(Vector3f(0.f, 0.f, 0.f), x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage: ./render <scene_config> <out_path> <interpolation variant>\n";
        return 1;
    }
    Scene scene(argv[1]);

    int option = atoi(argv[3]);

    Integrator rayTracer(scene);
    auto renderTime = rayTracer.render(option);
    
    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
