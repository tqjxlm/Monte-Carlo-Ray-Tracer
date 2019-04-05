#pragma once

#include <vector>
#include <functional>

#include <glm/glm.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Ray.hpp"
#include "Mesh.hpp"
#include "Triangle.h"

class Scene
{
public:
    Scene()
    {
    }

    ~Scene()
    {
        for (auto &rg : renderGroups)
        {
            for (auto triangle : rg.triangles)
            {
                delete triangle;
            }
        }

        for (auto m : materials)
        {
            delete m;
        }
    }

    void  initialize()
    {
        // Pre-store all emissive materials in a separate vector.
        for (auto &rg : renderGroups)
        {
            if (rg.material->isEmissive())
            {
                emissiveMesh.push_back(&rg);
            }
        }
    }

    const Mesh & getRenderGroup(unsigned renderGroupIndex) const
    {
        return renderGroups[renderGroupIndex];
    }

    const Triangle & getTriangle(unsigned int renderGroupIndex, unsigned int index) const
    {
        return *(renderGroups[renderGroupIndex].triangles[index]);
    }

    const std::vector<Mesh *> & getEmissiveMeshes() const
    {
        return emissiveMesh;
    }

    // Casts a ray through the scene. Save the closest intersection.
    bool  rayCast(const Ray &ray, unsigned int &intersectionRenderGroupIndex,
                  unsigned int &intersectionTriangleIndex, float &intersectionDistance) const;

    // Casts a ray through a given render group. Returns true if there was an intersection.
    bool  renderGroupRayCast(const Ray &ray, unsigned int renderGroupIndex,
                             unsigned int &intersectionTriangleIndex, float &intersectionDistance) const;

    void  addObj(std::string filePath,
                 glm::vec3 translate = glm::vec3(), glm::vec3 rotate = glm::vec3(), glm::vec3 scale = glm::vec3(1.0f));

    void  addMesh(const tinyobj::attrib_t &attrib,
                  const tinyobj::mesh_t &mesh, const std::vector<tinyobj::material_t> &materials,
                  const glm::mat4 &modelMatrix = glm::mat4());

private:
    std::vector<Mesh>        renderGroups;
    std::vector<Material *>  materials;
    std::vector<Mesh *>      emissiveMesh;
};
