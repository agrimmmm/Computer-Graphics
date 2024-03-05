#include "render.h"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}

Vector3f Integrator::uniformHemisphereSampling(Interaction si)
{
    int N = 1;
    Vector3f result(0, 0, 0);
    for (int i = 0; i < N; i++)
    {
        float phi = 2 * M_PI * next_float();
        float theta = std::acos(next_float());

        Vector3f point = si.p + 1e-5 * si.n;
        Vector3f u = Normalize(Vector3f(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta)));
        Vector3f direction = si.toWorld(u);

        Ray sampleRay = Ray(point, direction);
        Interaction sampleSi = this->scene.rayEmitterIntersect(sampleRay);
        Interaction sampleSi2 = this->scene.rayIntersect(sampleRay);
        if (sampleSi2.t < sampleSi.t && sampleSi2.didIntersect)
            continue;

        float costheta = Dot(direction, si.n) / (direction.Length() * si.n.Length());

        if (sampleSi.didIntersect)
            result += si.bsdf->eval(&si, u) * sampleSi.emissiveColor * costheta;
    }

    result = 2 * M_PI * result / N;
    // std::cout << result.x << " " << result.y << " " << result.z << std::endl;
    return result;
}

Vector3f Integrator::CosineWeightedSampling(Interaction si)
{
    int N = 1;
    Vector3f result(0, 0, 0);
    for (int i = 0; i < N; i++)
    {
        float r = std::sqrt(next_float());
        float theta = 2 * M_PI * next_float();

        Vector3f point = si.p + 1e-5 * si.n;
        Vector3f u;
        u.x = r * std::cos(theta);
        u.y = r * std::sin(theta);
        u.z = 1 - (u.x * u.x) - (u.y * u.y);
        if(u.z < 0)
            u.z = 0;
        Vector3f direction = si.toWorld(u);

        Ray sampleRay = Ray(point, direction);
        Interaction sampleSi = this->scene.rayEmitterIntersect(sampleRay);
        Interaction sampleSi2 = this->scene.rayIntersect(sampleRay);
        if (sampleSi2.t < sampleSi.t && sampleSi2.didIntersect)
            continue;

        if (sampleSi.didIntersect)
            result += si.bsdf->eval(&si, u) * sampleSi.emissiveColor;
    }
    result = M_PI * result / N;
    return result;
}

long long Integrator::render(int spp, int sampling_mode)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++)
    {
        for (int y = 0; y < this->scene.imageResolution.y; y++)
        {
            Vector3f result(0, 0, 0);
            for (int i = 0; i < spp; i++)
            {
                int flag = 0;
                Ray cameraRay = this->scene.camera.generateRay(x, y);
                Interaction si = this->scene.rayIntersect(cameraRay);
                Interaction altSi = this->scene.rayEmitterIntersect(cameraRay);
                if (altSi.didIntersect && altSi.t < si.t)
                {
                    si = altSi;
                    flag = 1;
                    result += altSi.emissiveColor;
                }

                if (si.didIntersect)
                {
                    Vector3f radiance;
                    LightSample ls;
                    for (Light &light : this->scene.lights)
                    {
                        std::tie(radiance, ls) = light.sample(&si);

                        Ray shadowRay(si.p + 1e-3f * si.n, ls.wo);
                        Interaction siShadow = this->scene.rayIntersect(shadowRay);

                        if (!siShadow.didIntersect || siShadow.t > ls.d)
                        {
                            if (flag == 0)
                            {
                                if (sampling_mode == 0)
                                    result += this->uniformHemisphereSampling(si);
                                else if (sampling_mode == 1)
                                    result += this->CosineWeightedSampling(si);
                                else if (sampling_mode == 2)
                                {
                                    float costheta = Dot(si.n, ls.wo) / (ls.wo.Length() * si.n.Length());
                                    result += si.bsdf->eval(&si, si.toLocal(ls.wo)) * radiance * costheta;
                                }
                                else
                                {
                                    printf("Invalid sampling mode\n");
                                    return 0;
                                }
                            }
                        }
                        // std::cout << count << std::endl;
                    }
                }
            }
            result = result / spp;
            this->outputImage.writePixelColor(result, x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        std::cerr << "Usage: ./render <scene_config> <out_path> <num_samples> <sampling_strategy>";
        return 1;
    }
    Scene scene(argv[1]);

    Integrator rayTracer(scene);
    int spp = atoi(argv[3]);
    int sampling_mode = atoi(argv[4]);
    auto renderTime = rayTracer.render(spp, sampling_mode);

    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
