#pragma once

#include <vector>
#include <memory>

#include "Material.hpp"
#include "Triangle.h"
#include "KDTree.h"

class Scene;
class Renderer;

struct ObjectIntersection
{
    bool   hit; // If there was an intersection
    float  dist; // Distance to intersection along ray
    long   index; // Hit triangle index

    ObjectIntersection(bool hit_ = false, float dist_ = 0, long index_ = 0):
        hit(hit_), dist(dist_), index(index_)
    {
    }
};

class Mesh
{
public:
    Mesh(Material *mat):
        material(mat)
    {
    }

    glm::vec3  getRandomPositionOnSurface() const
    {
        const auto  triangle = triangles[rand() % triangles.size()];

        return triangle->getRandomPositionOnSurface();
    }

    ObjectIntersection  getIntersection(const Ray &ray) const
    {
        float      t = 0, tmin = INFINITY;
        glm::vec3  colour = glm::vec3();
        long       index  = 0;
        bool       hit    = node->hit(node, ray, t, tmin, index);

        return ObjectIntersection(hit, tmin, index);
    }

private:
    bool                     enabled = true;
    bool                     convex  = true;
    Material                *material;
    std::vector<Triangle *>  triangles;
    KDNode                  *node;

    friend Scene;
    friend Renderer;
};
