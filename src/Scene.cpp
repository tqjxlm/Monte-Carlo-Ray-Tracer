#include "Scene.h"

#include <cassert>
#include <limits>
#include <exception>
#include <iostream>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Material.hpp"

inline glm::vec3 toVec3(const std::vector<tinyobj::real_t>& numbers)
{
    return glm::vec3(numbers[0], numbers[1], numbers[2]);
}

inline glm::vec3 toVec3(const tinyobj::real_t numbers[])
{
    return glm::vec3(numbers[0], numbers[1], numbers[2]);
}

inline glm::vec3 getFace(const tinyobj::attrib_t& attrib,
                         const tinyobj::mesh_t  & mesh,
                         int                      f)
{
    return glm::vec3(
        attrib.vertices[mesh.indices[f].vertex_index * 3],
        attrib.vertices[mesh.indices[f].vertex_index * 3 + 1],
        attrib.vertices[mesh.indices[f].vertex_index * 3 + 2]);
}

inline glm::vec3 getFace(const tinyobj::attrib_t& attrib,
                         const tinyobj::mesh_t  & mesh,
                         const glm::mat4        & modelMatrix,
                         int                      f)
{
    return glm::vec3(modelMatrix * glm::vec4(getFace(attrib, mesh, f), 1.0f));
}

inline glm::vec3 getNormal(const tinyobj::attrib_t& attrib,
                           const tinyobj::mesh_t  & mesh,
                           int                      f)
{
    if (mesh.indices[f].normal_index == -1)
    {
        return glm::vec3();
    }

    return glm::normalize(glm::vec3(
                              attrib.normals[mesh.indices[f].normal_index * 3],
                              attrib.normals[mesh.indices[f].normal_index * 3 + 1],
                              attrib.normals[mesh.indices[f].normal_index * 3 + 2]));
}

inline glm::vec3 getNormal(const tinyobj::attrib_t& attrib,
                           const tinyobj::mesh_t  & mesh,
                           const glm::mat4        & modelMatrix,
                           int                      f)
{
    if (mesh.indices[f].normal_index == -1)
    {
        return glm::vec3();
    }

    return glm::mat3(modelMatrix) * getNormal(attrib, mesh, f);
}

inline glm::vec3 getFaceNormal(const tinyobj::attrib_t& attrib,
                               const tinyobj::mesh_t  & mesh,
                               int                      f)
{
    return glm::normalize((getNormal(attrib, mesh, f) +
                           getNormal(attrib, mesh, f + 1) +
                           getNormal(attrib, mesh, f + 2)) /
                          3.0f);
}

inline LambertianMaterial* toLambertianMaterial(const tinyobj::material_t* material)
{
    LambertianMaterial* meshMaterial;
    float opacity  = material->dissolve;
    float reflect  = (material->ambient[0] + material->ambient[1] + material->ambient[2]) / 3;
    float specular = (material->specular[0] + material->specular[1] + material->specular[2]) / 3;
    float emission = (material->emission[0] + material->emission[1] + material->emission[2]) / 3;

    if ((material->emission[0] > 0) || (material->emission[1] > 0) || (material->emission[2] > 0))
    {
        meshMaterial = new LambertianMaterial(toVec3(material->diffuse), emission);
    }
    else
    {
        meshMaterial = new LambertianMaterial(toVec3(material->diffuse),
                                              0.0f,
                                              reflect,
                                              1 - opacity,
                                              material->ior,
                                              specular,
                                              material->shininess);
    }

    return meshMaterial;
}

bool Scene::rayCast(const Ray   & ray,
                    unsigned int& intersectionRenderGroupIndex,
                    unsigned int& intersectionTriangleIndex,
                    float       & intersectionDistance) const
{
    float closestInterectionDistance = std::numeric_limits<float>::max();

    // TODO: Use a BVH to avoid traversal
    for (unsigned int i = 0; i < renderGroups.size(); ++i)
    {
        if (!renderGroups[i].enabled)
        {
            continue;
        }

        ObjectIntersection intersection = renderGroups[i].getIntersection(ray);

        if (intersection.hit)
        {
            intersectionDistance = intersection.dist;

            if (intersectionDistance < closestInterectionDistance)
            {
                intersectionRenderGroupIndex = i;
                intersectionTriangleIndex    = intersection.index;
                closestInterectionDistance   = intersectionDistance;
            }
        }
    }

    intersectionDistance = closestInterectionDistance;

    return closestInterectionDistance <
           std::numeric_limits<float>::max() - std::numeric_limits<float>::min();
}

bool Scene::renderGroupRayCast(const Ray   & ray,
                               unsigned int  renderGroupIndex,
                               unsigned int& intersectionTriangleIndex,
                               float       & intersectionDistance) const
{
    float closestInterectionDistance = std::numeric_limits<float>::max();
    const auto& renderGroup          = renderGroups[renderGroupIndex];

    for (unsigned int j = 0; j < renderGroup.triangles.size(); ++j)
    {
        if (!renderGroup.triangles[j]->enabled)
        {
            continue;
        }

        bool intersects = renderGroup.triangles[j]->rayIntersection(ray,
                                                                    intersectionDistance);

        if (intersects)
        {
            assert(intersectionDistance > std::numeric_limits<float>::min());

            if (intersectionDistance < closestInterectionDistance)
            {
                intersectionTriangleIndex  = j;
                closestInterectionDistance = intersectionDistance;
            }
        }
    }

    intersectionDistance = closestInterectionDistance;

    return closestInterectionDistance <
           std::numeric_limits<float>::max() - std::numeric_limits<float>::min();
}

void Scene::addObj(std::string filePath,
                   glm::vec3   translate,
                   glm::vec3   rotate,
                   glm::vec3   scale)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string warn;
    std::string mtlbasepath;
    size_t pos = filePath.find_last_of("/");

    mtlbasepath = filePath.substr(0, pos + 1);

    bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str(), mtlbasepath.c_str());
    glm::mat4 modelMatrix;
    modelMatrix = glm::translate(modelMatrix, translate);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.x), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.y), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.z), glm::vec3(0, 0, 1));

    if (!warn.empty())
    {
        std::cout << warn << std::endl;
    }
    if (!err.empty())
    {
        std::cout << err << std::endl;
    }

    if (!res)
    {
        throw std::exception();
    }
    else
    {
        for (const auto& shape : shapes)
        {
            addMesh(attrib, shape.mesh, materials, modelMatrix);
        }
    }
}

void Scene::addMesh(const tinyobj::attrib_t               & attrib,
                    const tinyobj::mesh_t                 & mesh,
                    const std::vector<tinyobj::material_t>& objMaterials,
                    const glm::mat4                       & modelMatrix)
{
    const tinyobj::material_t* currentMaterial = &objMaterials[mesh.material_ids[0]];

    // Transform obj mtl to LambertianMeterial
    LambertianMaterial* meshMaterial = toLambertianMaterial(currentMaterial);

    materials.push_back(meshMaterial);

    // New render group
    Mesh meshGroup(meshMaterial);

    for (size_t i = 0; i < mesh.num_face_vertices.size(); i++)
    {
        int vertOffset = (int)i * 3;
        Triangle* tri  = new Triangle(getFace(attrib, mesh, modelMatrix, vertOffset),
                                      getFace(attrib, mesh, modelMatrix, vertOffset + 1),
                                      getFace(attrib, mesh, modelMatrix, vertOffset + 2),
                                      getNormal(attrib, mesh, modelMatrix, vertOffset),
                                      getNormal(attrib, mesh, modelMatrix, vertOffset + 1),
                                      getNormal(attrib, mesh, modelMatrix, vertOffset + 2),
                                      (int)meshGroup.triangles.size());
        meshGroup.triangles.push_back(tri);
    }

    meshGroup.node = KDNode().build(meshGroup.triangles, 0);

    renderGroups.push_back(meshGroup);
}
