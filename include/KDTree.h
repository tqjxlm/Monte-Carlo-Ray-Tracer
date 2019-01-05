#ifndef KDTREE_H
#define KDTREE_H

#include <vector>

#include "Triangle.h"
#include "Ray.hpp"
#include "AABB.hpp"

class KDNode
{
public:
    KDNode()
    {
    }

    KDNode* build(std::vector<Triangle *> &tris, int depth);

    bool    hit(KDNode *node, const Ray &ray, float &t, float &tmin, long &tri_idx);

private:
    AABB                     box;
    KDNode                  *left;
    KDNode                  *right;
    std::vector<Triangle *>  triangles;
    bool                     leaf;
};

#endif // KDTREE_H
