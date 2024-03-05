#include "surface.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

std::vector<Surface> createSurfaces(std::string pathToObj, bool isLight, uint32_t shapeIdx)
{
    std::string objDirectory;
    const size_t last_slash_idx = pathToObj.rfind('/');
    if (std::string::npos != last_slash_idx) {
        objDirectory = pathToObj.substr(0, last_slash_idx);
    }

    std::vector<Surface> surfaces;

    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    if (!reader.ParseFromFile(pathToObj, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        Surface surf;
        surf.triangles = (Triangles*)calloc(sizeof(struct Triangles), INT16_MAX);
        surf.isLight = isLight;
        surf.shapeIdx = shapeIdx;
        std::set<int> materialIds;

        surf.tricount = 0;
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            if (fv > 3) {
                std::cerr << "Not a triangle mesh" << std::endl;
                exit(1);
            }

            // Loop over vertices in the face. Assume 3 vertices per-face
            Vector3f vertices[3], normals[3];
            Vector2f uvs[3];
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                    normals[v] = Vector3f(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                    uvs[v] = Vector2f(tx, ty);
                }

                vertices[v] = Vector3f(vx, vy, vz);
            }

            int vSize = surf.vertices.size();
            Vector3i findex(vSize, vSize + 1, vSize + 2);

            vertices[0] -= camFrom;
            vertices[1] -= camFrom;
            vertices[2] -= camFrom;

            surf.triangles[surf.tricount].v1 = vertices[0];
            surf.triangles[surf.tricount].v2 = vertices[1];
            surf.triangles[surf.tricount].v3 = vertices[2];

            surf.triangles[surf.tricount].n1 = normals[0];
            surf.triangles[surf.tricount].n2 = normals[1];
            surf.triangles[surf.tricount].n3 = normals[2];

            surf.triangles[surf.tricount].aabbmax.x = std::max(vertices[0].x, std::max(vertices[1].x, vertices[2].x));
            surf.triangles[surf.tricount].aabbmax.y = std::max(vertices[0].y, std::max(vertices[1].y, vertices[2].y));
            surf.triangles[surf.tricount].aabbmax.z = std::max(vertices[0].z, std::max(vertices[1].z, vertices[2].z));
            surf.triangles[surf.tricount].aabbmin.x = std::min(vertices[0].x, std::min(vertices[1].x, vertices[2].x));
            surf.triangles[surf.tricount].aabbmin.y = std::min(vertices[0].y, std::min(vertices[1].y, vertices[2].y));
            surf.triangles[surf.tricount].aabbmin.z = std::min(vertices[0].z, std::min(vertices[1].z, vertices[2].z));

            surf.triangles[surf.tricount].centroid.x = (vertices[0].x + vertices[1].x + vertices[2].x) / 3;
            surf.triangles[surf.tricount].centroid.y = (vertices[0].y + vertices[1].y + vertices[2].y) / 3;
            surf.triangles[surf.tricount].centroid.z = (vertices[0].z + vertices[1].z + vertices[2].z) / 3;

            surf.tricount++;

            surf.vertices.push_back(vertices[0]);
            surf.vertices.push_back(vertices[1]);
            surf.vertices.push_back(vertices[2]);

            surf.normals.push_back(normals[0]);
            surf.normals.push_back(normals[1]);
            surf.normals.push_back(normals[2]);

            surf.uvs.push_back(uvs[0]);
            surf.uvs.push_back(uvs[1]);
            surf.uvs.push_back(uvs[2]);

            surf.indices.push_back(findex);

            // per-face material
            materialIds.insert(shapes[s].mesh.material_ids[f]);

            index_offset += fv;
        }

        // std::cout << surf.vertices.size() << std::endl;
        // std::cout << surf.indices.size() << std::endl;
        // for(int i=0; i<surf.vertices.size(); i+=3)
        // {
        //     surf.triangles[surf.tricount].aabbmax.x = std::max(surf.vertices[i].x, std::max(surf.vertices[i+1].x, surf.vertices[i+2].x));
        //     surf.triangles[surf.tricount].aabbmax.y = std::max(surf.vertices[i].y, std::max(surf.vertices[i+1].y, surf.vertices[i+2].y));
        //     surf.triangles[surf.tricount].aabbmax.z = std::max(surf.vertices[i].z, std::max(surf.vertices[i+1].z, surf.vertices[i+2].z));
        //     surf.triangles[surf.tricount].aabbmin.x = std::min(surf.vertices[i].x, std::min(surf.vertices[i+1].x, surf.vertices[i+2].x));
        //     surf.triangles[surf.tricount].aabbmin.y = std::min(surf.vertices[i].y, std::min(surf.vertices[i+1].y, surf.vertices[i+2].y));
        //     surf.triangles[surf.tricount].aabbmin.z = std::min(surf.vertices[i].z, std::min(surf.vertices[i+1].z, surf.vertices[i+2].z));
        //     surf.triangles[surf.tricount].aabbcentre.x = (surf.triangles[surf.tricount].aabbmin.x + surf.triangles[surf.tricount].aabbmax.x) / 2;
        //     surf.triangles[surf.tricount].aabbcentre.y = (surf.triangles[surf.tricount].aabbmin.y + surf.triangles[surf.tricount].aabbmax.y) / 2;
        //     surf.triangles[surf.tricount].aabbcentre.z = (surf.triangles[surf.tricount].aabbmin.z + surf.triangles[surf.tricount].aabbmax.z) / 2;

        //     surf.triangles[surf.tricount].v1 = surf.vertices[i];
        //     surf.triangles[surf.tricount].v2 = surf.vertices[i+1];
        //     surf.triangles[surf.tricount].v3 = surf.vertices[i+2];

        //     surf.triangles[surf.tricount].n1 = surf.normals[i];
        //     surf.triangles[surf.tricount].n2 = surf.normals[i+1];
        //     surf.triangles[surf.tricount].n3 = surf.normals[i+2];

        //     surf.tricount++;
        // } 
        // std::cout << surf.tricount << std::endl;

        //Initialise coordinates of AABB
        surf.aabbmin = Vector3f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        surf.aabbmax = Vector3f(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

        // Find coordinates of AABB
        for (auto v : surf.vertices) {
            surf.aabbmin = Vector3f(std::min(surf.aabbmin.x, v.x), std::min(surf.aabbmin.y, v.y), std::min(surf.aabbmin.z, v.z));
            surf.aabbmax = Vector3f(std::max(surf.aabbmax.x, v.x), std::max(surf.aabbmax.y, v.y), std::max(surf.aabbmax.z, v.z));
            surf.aabbcentre.x = (surf.aabbmin.x + surf.aabbmax.x) / 2;
            surf.aabbcentre.y = (surf.aabbmin.y + surf.aabbmax.y) / 2;
            surf.aabbcentre.z = (surf.aabbmin.z + surf.aabbmax.z) / 2;
        }

        if (materialIds.size() > 1) {
            std::cerr << "One of the meshes has more than one material. This is not allowed." << std::endl;
            exit(1);
        }


        if (materialIds.size() == 0) {
            std::cerr << "One of the meshes has no material definition, may cause unexpected behaviour." << std::endl;
        }
        else {
            // Load textures from Materials
            auto matId = *materialIds.begin();
            if (matId != -1) {
                auto mat = materials[matId];

                surf.diffuse = Vector3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
                if (mat.diffuse_texname != "")
                    surf.diffuseTexture = Texture(objDirectory + "/" + mat.diffuse_texname);

                surf.alpha = mat.specular[0];
                if (mat.alpha_texname != "")
                    surf.alphaTexture = Texture(objDirectory + "/" + mat.alpha_texname);
            }
        }

        surfaces.push_back(surf);
        shapeIdx++;
    }

    return surfaces;
}

bool Surface::hasDiffuseTexture() { return this->diffuseTexture.data != 0; }

bool Surface::hasAlphaTexture() { return this->alphaTexture.data != 0; }

Interaction Surface::rayPlaneIntersect(Ray ray, Vector3f p, Vector3f n)
{
    Interaction si;

    float dDotN = Dot(ray.d, n);
    if (dDotN != 0.f) {
        float t = -Dot((ray.o - p), n) / dDotN;

        if (t >= 0.f) {
            si.didIntersect = true;
            si.t = t;
            si.n = n;
            si.p = ray.o + ray.d * si.t;
        }
    }

    return si;
}

bool Surface::checktriaabbinter(Ray& ray, Triangles& triangle)
{
    float tx1 = (triangle.aabbmin.x - ray.o.x) / ray.d.x, tx2 = (triangle.aabbmax.x - ray.o.x) / ray.d.x;
    float tmin = std::min(tx1, tx2), tmax = std::max(tx1, tx2);
    float ty1 = (triangle.aabbmin.y - ray.o.y) / ray.d.y, ty2 = (triangle.aabbmax.y - ray.o.y) / ray.d.y;
    tmin = std::max(tmin, std::min(ty1, ty2)), tmax = std::min(tmax, std::max(ty1, ty2));
    float tz1 = (triangle.aabbmin.z - ray.o.z) / ray.d.z, tz2 = (triangle.aabbmax.z - ray.o.z) / ray.d.z;
    tmin = std::max(tmin, std::min(tz1, tz2)), tmax = std::min(tmax, std::max(tz1, tz2));
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

bool Surface::checktribvhinter(Ray& ray, triBVH* root)
{
    // std::cout << root->bvhmax.x << std::endl;
    float tx1 = (root->bvhmin.x - ray.o.x) / ray.d.x, tx2 = (root->bvhmax.x - ray.o.x) / ray.d.x;
    float tmin = std::min(tx1, tx2), tmax = std::max(tx1, tx2);
    float ty1 = (root->bvhmin.y - ray.o.y) / ray.d.y, ty2 = (root->bvhmax.y - ray.o.y) / ray.d.y;
    tmin = std::max(tmin, std::min(ty1, ty2)), tmax = std::min(tmax, std::max(ty1, ty2));
    float tz1 = (root->bvhmin.z - ray.o.z) / ray.d.z, tz2 = (root->bvhmax.z - ray.o.z) / ray.d.z;
    tmin = std::max(tmin, std::min(tz1, tz2)), tmax = std::min(tmax, std::max(tz1, tz2));
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

Interaction Surface::aabbRayIntersect(Ray& ray, triBVH* triroot)
{
    Interaction siFinal;
    float tmin = ray.t;

    // std::cout << triroot->surface->triangles[0].aabbmax.x << std::endl;

    // std::cout << "I'm here" << std::endl;
    // std::cout << triroot->bvhmax.x << std::endl;
    if(!checktribvhinter(ray, triroot))
        return siFinal;
    if(triroot->left == NULL && triroot->right == NULL)
    {
        for(int i=0; i<triroot->count; i++)
        {
            // std::cout << "Fine till here" << std::endl;
            if(!checktriaabbinter(ray, triroot->triangles[i]))
                continue;
            Vector3f p1 = triroot->triangles[i].v1;
            Vector3f p2 = triroot->triangles[i].v2;
            Vector3f p3 = triroot->triangles[i].v3;

            Vector3f n1 = triroot->triangles[i].n1;
            Vector3f n2 = triroot->triangles[i].n2;
            Vector3f n3 = triroot->triangles[i].n3;
            Vector3f n = Normalize(n1 + n2 + n3);

            Interaction si = this->rayTriangleIntersect(ray, p1, p2, p3, n);
            if (si.t <= tmin && si.didIntersect) {
                siFinal = si;
                tmin = si.t;
            }
        }
        return siFinal;
    }
    else
    {
        // std::cout << "yahan pe" << std::endl;
        // return aabbRayIntersect_contd(ray, triroot, tmin);
        Interaction l = aabbRayIntersect(ray, triroot->left);
        Interaction r = aabbRayIntersect(ray, triroot->right);
        if(l.t <= r.t)
            return l;
        else
            return r;
    }
}

Interaction Surface::rayTriangleIntersect(Ray ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f n)
{
    Interaction si = this->rayPlaneIntersect(ray, v1, n);

    if (si.didIntersect) {
        bool edge1 = false, edge2 = false, edge3 = false;

        // Check edge 1
        {
            Vector3f nIp = Cross((si.p - v1), (v3 - v1));
            Vector3f nTri = Cross((v2 - v1), (v3 - v1));
            edge1 = Dot(nIp, nTri) > 0;
        }

        // Check edge 2
        {
            Vector3f nIp = Cross((si.p - v1), (v2 - v1));
            Vector3f nTri = Cross((v3 - v1), (v2 - v1));
            edge2 = Dot(nIp, nTri) > 0;
        }

        // Check edge 3
        {
            Vector3f nIp = Cross((si.p - v2), (v3 - v2));
            Vector3f nTri = Cross((v1 - v2), (v3 - v2));
            edge3 = Dot(nIp, nTri) > 0;
        }

        if (edge1 && edge2 && edge3) {
            // Intersected triangle!
            si.didIntersect = true;
        }
        else {
            si.didIntersect = false;
        }
    }

    return si;
}

Interaction Surface::rayIntersect(Ray ray)
{
    Interaction siFinal;
    float tmin = ray.t;

    for (auto face : this->indices) {
        Vector3f p1 = this->vertices[face.x];
        Vector3f p2 = this->vertices[face.y];
        Vector3f p3 = this->vertices[face.z];

        Vector3f n1 = this->normals[face.x];
        Vector3f n2 = this->normals[face.y];
        Vector3f n3 = this->normals[face.z];
        Vector3f n = Normalize(n1 + n2 + n3);

        Interaction si = this->rayTriangleIntersect(ray, p1, p2, p3, n);
        if (si.t <= tmin && si.didIntersect) {
            siFinal = si;
            tmin = si.t;
        }
    }

    return siFinal;
}