#include <vector>

#include "KDTree.h"

// Build KD tree for tris
KDNode * KDNode::build(std::vector<Triangle *> &tris, int depth)
{
    KDNode *node = new KDNode();

    node->leaf      = false;
    node->triangles = std::vector<Triangle *>();
    node->left      = nullptr;
    node->right     = nullptr;
    node->box       = AABB();

    if (tris.size() == 0) { return node; }

    if ((depth > 25) || (tris.size() <= 6))
    {
        node->triangles = tris;
        node->leaf      = true;
        node->box       = tris[0]->getBoundingBox();

        for (long i = 1; i < tris.size(); i++)
        {
            node->box.expand(tris[i]->getBoundingBox());
        }

        node->left             = new KDNode();
        node->right            = new KDNode();
        node->left->triangles  = std::vector<Triangle *>();
        node->right->triangles = std::vector<Triangle *>();

        return node;
    }

    node->box = tris[0]->getBoundingBox();
    glm::vec3  midpt     = glm::vec3();
    float      tris_recp = 1.0f / tris.size();

    for (long i = 1; i < tris.size(); i++)
    {
        node->box.expand(tris[i]->getBoundingBox());
        midpt = midpt + (tris[i]->getCenter() * tris_recp);
    }

    std::vector<Triangle *>  left_tris;
    std::vector<Triangle *>  right_tris;
    int                      axis = node->box.get_longest_axis();

    for (auto tri : tris)
    {
        switch (axis)
        {
        case 0:
            midpt.x >= tri->getCenter().x ? right_tris.push_back(tri) : left_tris.push_back(tri);
            break;
        case 1:
            midpt.y >= tri->getCenter().y ? right_tris.push_back(tri) : left_tris.push_back(tri);
            break;
        case 2:
            midpt.z >= tri->getCenter().z ? right_tris.push_back(tri) : left_tris.push_back(tri);
            break;
        }
    }

    if ((tris.size() == left_tris.size()) || (tris.size() == right_tris.size()))
    {
        node->triangles = tris;
        node->leaf      = true;
        node->box       = tris[0]->getBoundingBox();

        for (long i = 1; i < tris.size(); i++)
        {
            node->box.expand(tris[i]->getBoundingBox());
        }

        node->left             = new KDNode();
        node->right            = new KDNode();
        node->left->triangles  = std::vector<Triangle *>();
        node->right->triangles = std::vector<Triangle *>();

        return node;
    }

    node->left  = build(left_tris, depth + 1);
    node->right = build(right_tris, depth + 1);

    return node;
}

// Finds nearest triangle in kd tree that intersects with ray.
bool  KDNode::hit(KDNode *node, const Ray &ray, float &t, float &tmin, long &tri_idx)
{
    float  dist;

    if (node->box.intersection(ray, dist))
    {
        if (dist > tmin) { return false; }

        bool  hit_tri   = false;
        bool  hit_left  = false;
        bool  hit_right = false;

        if (!node->leaf)
        {
            hit_left  = hit(node->left, ray, t, tmin, tri_idx);
            hit_right = hit(node->right, ray, t, tmin, tri_idx);

            return hit_left || hit_right;
        }
        else
        {
            auto  triangles_size = node->triangles.size();

            for (size_t i = 0; i < triangles_size; i++)
            {
                if (node->triangles[i]->rayIntersection(ray, t) && (t < tmin))
                {
                    hit_tri = true;
                    tmin    = t;
                    tri_idx = node->triangles[i]->meshIndex;
                }
            }

            return hit_tri;
        }
    }

    return false;
}
